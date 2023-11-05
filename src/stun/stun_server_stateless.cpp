//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Stateless server implementation
//

#include "stun/stun_server_stateless.hpp"
#include "util/util_variant_overloaded.hpp"
#include "util/util_fmap.hpp"

namespace freewebrtc::stun::server {

namespace {

using MaybeStringView = std::optional<std::string_view>;
Message create_error(const Message& msg, ErrorCodeAttribute::Code error, MaybeStringView reason = std::nullopt) {
    Header header {
        Class::error_response(),
        msg.header.method,
        TransactionId(msg.header.transaction_id.view())
    };
    AttributeSet attrset;
    attrset.emplace(
        Attribute::create(
            ErrorCodeAttribute{
                error,
                util::fmap(
                    reason,
                    [](const auto& a) -> std::string {
                        return std::string{a};
                    })
            }
        )
    );
    return Message {
        std::move(header),
        attrset,
        msg.is_rfc3489
    };
}

}

Stateless::Stateless(crypto::SHA1Hash::Func sha1, const std::optional<Settings>& maybe_settings)
    : m_sha1(sha1)
    , m_settings(maybe_settings.value_or(Settings{}))
{}

Stateless::ProcessResult Stateless::process(const net::Endpoint& ep, const util::ConstBinaryView& view) {
    auto maybe_msg = stun::Message::parse(view, m_stat);
    if (!maybe_msg.has_value()) {
        return Ignore{};
    }
    auto& msg = maybe_msg.value();
    // RFC 5389: 7.3.1. Processing a Request
    // If the request contains one or more unknown comprehension-required
    // attributes, the server replies with an error response with an error
    // code of 420 (Unknown Attribute), and includes an UNKNOWN-ATTRIBUTES
    // attribute in the response that lists the unknown comprehension-
    // required attributes.
    if (msg.header.cls == stun::Class::request()) {
        return process_request(ep, std::move(msg), view);
    }
    return Ignore{};
}

void Stateless::add_user(const precis::OpaqueString& name, const stun::Password& password) {
    m_users.emplace(name, password);
}

Stateless::ProcessResult Stateless::process_request(const net::Endpoint& ep, Message&& msg, const util::ConstBinaryView& view) {
    const auto unknown_comprehension_required = msg.attribute_set.unknown_comprehension_required();
    if (!unknown_comprehension_required.empty()) {
        auto rsp = create_error(msg, ErrorCodeAttribute::Code::UnknownAttribute);
        auto attr = Attribute::create(UnknownAttributesAttribute{std::move(unknown_comprehension_required)});
        rsp.attribute_set.emplace(std::move(attr));
        return Respond{rsp, std::move(msg)};
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
        return Respond{create_error(msg, ErrorCodeAttribute::Code::BadRequest), std::move(msg)};
    }

    std::optional<IntegrityData> maybe_integrity_data;
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
            return Respond{create_error(msg, ErrorCodeAttribute::Code::Unauthorized), std::move(msg)};
        }
        // Using the password associated with the username, compute the value
        // for the message integrity as described in Section 15.4.  If the
        // resulting value does not match the contents of the MESSAGE-
        // INTEGRITY attribute:
        //
        // *  If the message is a request, the server MUST reject the request
        //    with an error response.  This response MUST use an error code
        //    of 401 (Unauthorized).
        maybe_integrity_data = IntegrityData{user_password_pair_it->second, m_sha1};
        const auto maybe_is_valid_rv = msg.is_valid(view, maybe_integrity_data.value());
        if (maybe_is_valid_rv.is_error()) {
            return Error{maybe_is_valid_rv.assert_error()};
        }
        const auto maybe_is_valid = maybe_is_valid_rv.assert_value();
        if (maybe_is_valid.has_value() && !maybe_is_valid.value()) {
            return Respond{create_error(msg, ErrorCodeAttribute::Code::Unauthorized), std::move(msg)};
        }
    }

    if (msg.header.method == Method::binding()) {
        Header header {
            Class::success_response(),
            msg.header.method,
            TransactionId(msg.header.transaction_id.view())
        };
        AttributeSet attrset;
        if (!msg.is_rfc3489 && m_settings.use_fingerprint) {
            attrset.emplace(Attribute::create(FingerprintAttribute{0}));
        }
        if (!msg.is_rfc3489) {
            auto xored_addr = XoredAddress::from_address(ep.address(), msg.header.transaction_id);
            attrset.emplace(Attribute::create(XorMappedAddressAttribute{xored_addr, ep.port()}));
        } else {
            // Normative:
            // When the server detects an RFC 3489 client, it SHOULD
            // copy the value seen in the magic cookie field in the
            // Binding request to the magic cookie field in the
            // Binding response message, and1 insert a MAPPED-ADDRESS
            // attribute instead of an XOR-MAPPED-ADDRESS attribute.
            attrset.emplace(Attribute::create(MappedAddressAttribute{ep.address(), ep.port()}));
        }
        return Respond{
            Message {
                std::move(header),
                attrset,
                msg.is_rfc3489
            },
            std::move(msg),
            maybe_integrity_data
        };
    }
    return Ignore{};
}

}

