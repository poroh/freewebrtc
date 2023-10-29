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
        std::move(attrset),
        msg.is_rfc3489
    };
}

}

std::optional<Message> Stateless::create_response(const net::Endpoint&, const Message& msg) {
    // RFC 5389: 7.3.1. Processing a Request
    // If the request contains one or more unknown comprehension-required
    // attributes, the server replies with an error response with an error
    // code of 420 (Unknown Attribute), and includes an UNKNOWN-ATTRIBUTES
    // attribute in the response that lists the unknown comprehension-
    // required attributes.
    const auto unknown_comprehension_required = msg.attribute_set.unknown_comprehension_required();
    if (!unknown_comprehension_required.empty()) {
        auto rsp = create_error(msg, ErrorCodeAttribute::Code::UnknownAttribute);
        auto attr = Attribute::create(UnknownAttributesAttribute{std::move(unknown_comprehension_required)});
        rsp.attribute_set.emplace(std::move(attr));
        return rsp;
    }
    return std::nullopt;
}

}
