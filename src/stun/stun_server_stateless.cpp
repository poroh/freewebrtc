//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Stateless server implementation
//

#include "stun/stun_server_stateless.hpp"

namespace freewebrtc::stun::server {

namespace {

Message create_error(const Message& msg, ErrorCodeAttribute::Code error) {
    Header header {
        Class::error_response(),
        msg.header.method,
        TransactionId(msg.header.transaction_id.view())
    };
    AttributeSet attrset;
    attrset.emplace(Attribute::create(ErrorCodeAttribute{error}));
    return Message {
        std::move(header),
        attrset,
        msg.is_rfc3489
    };
}

}

Stateless::Stateless(crypto::SHA1Hash::Func sha1)
    : m_sha1(sha1)
{}

Stateless::MaybeResponse Stateless::process(const net::Endpoint& ep, const util::ConstBinaryView& view) {
    const auto maybe_msg = stun::Message::parse(view, m_stat);
    if (!maybe_msg.has_value()) {
        return std::nullopt;
    }
    const auto& msg = maybe_msg.value();
    // RFC 5389: 7.3.1. Processing a Request
    // If the request contains one or more unknown comprehension-required
    // attributes, the server replies with an error response with an error
    // code of 420 (Unknown Attribute), and includes an UNKNOWN-ATTRIBUTES
    // attribute in the response that lists the unknown comprehension-
    // required attributes.
    if (msg.header.cls == stun::Class::request()) {
        return process_request(ep, msg, view);
    }
    return std::nullopt;
}

Stateless::MaybeResponse Stateless::process_request(const net::Endpoint&, const Message& msg, const util::ConstBinaryView& view) {
    const auto unknown_comprehension_required = msg.attribute_set.unknown_comprehension_required();
    if (!unknown_comprehension_required.empty()) {
        auto rsp = create_error(msg, ErrorCodeAttribute::Code::UnknownAttribute);
        auto attr = Attribute::create(UnknownAttributesAttribute{std::move(unknown_comprehension_required)});
        rsp.attribute_set.emplace(std::move(attr));
        return rsp;
    }
    // RFC5389 10.1.2. Receiving a Request or Indication
    auto maybe_username = msg.attribute_set.username();
    auto maybe_integrity = msg.attribute_set.integrity();
    if (maybe_username.has_value() != maybe_integrity.has_value()) {
        // If the message does not contain both a MESSAGE-INTEGRITY and a
        // USERNAME attribute:
        //
        // *  If the message is a request, the server MUST reject the request
        //    with an error response.  This response MUST use an error code
        //    of 400 (Bad Request).
        return create_error(msg, ErrorCodeAttribute::Code::BadRequest);
    }

    if (maybe_username.has_value()) {
        const auto& username = maybe_username->get();
        const auto user_password_pair_it = m_users.find(username);
        if (user_password_pair_it == m_users.end()) {
            // If the USERNAME does not contain a username value currently valid
            // within the server:
            //
            // *  If the message is a request, the server MUST reject the request
            //    with an error response.  This response MUST use an error code
            //    of 401 (Unauthorized).
            return create_error(msg, ErrorCodeAttribute::Code::Unauthorized);
        }
        // Using the password associated with the username, compute the value
        // for the message integrity as described in Section 15.4.  If the
        // resulting value does not match the contents of the MESSAGE-
        // INTEGRITY attribute
        msg.is_valid(view, IntegrityData{user_password_pair_it->second, m_sha1});
    }
    return std::nullopt;
}

}

