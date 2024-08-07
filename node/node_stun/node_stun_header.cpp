//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN to node.js type
//

#include <craftnapi/error.hpp>

#include "stun/stun_header.hpp"
#include "node/node_stun/node_stun_header.hpp"
#include "node/node_stun/node_stun_error.hpp"

namespace freewebrtc::node_stun {

Result<craftnapi::Value> stun_class(const craftnapi::Env& env, const stun::Class& cls) {
    switch (cls.value()) {
    case stun::Class::REQUEST:          return env.create_string("request");
    case stun::Class::INDICATION:       return env.create_string("indication");
    case stun::Class::SUCCESS_RESPONSE: return env.create_string("success_response");
    case stun::Class::ERROR_RESPONSE:   return env.create_string("error_response");
    }
    return make_error_code(Error::unknown_stun_class);
}

Result<craftnapi::Value> stun_method(const craftnapi::Env& env, const stun::Method& method) {
    if (method == stun::Method::binding()) {
        return env.create_string("binding");
    }
    return make_error_code(Error::unknown_stun_method);
}

Result<craftnapi::Value> stun_transcation_id(const craftnapi::Env& env, const stun::TransactionId& id) {
    return env.create_buffer(id.view());
}

Result<craftnapi::Object> header(const craftnapi::Env& env, const stun::Header& hdr) noexcept {
    return
        env.create_object({
                { "class", stun_class(env, hdr.cls) },
                { "method", stun_method(env, hdr.method) },
                { "transaction", stun_transcation_id(env, hdr.transaction_id) }
            })
        .add_context("stun header");
}

}
