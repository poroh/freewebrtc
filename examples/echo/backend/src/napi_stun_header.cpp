//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN to node.js type
//

#include "stun/stun_header.hpp"
#include "napi_stun_header.hpp"
#include "napi_error.hpp"

namespace freewebrtc::napi {

ReturnValue<Value> stun_class(const Env& env, const stun::Class& cls) {
    switch (cls.value()) {
    case stun::Class::REQUEST:          return env.create_string("request");
    case stun::Class::INDICATION:       return env.create_string("indication");
    case stun::Class::SUCCESS_RESPONSE: return env.create_string("success_response");
    case stun::Class::ERROR_RESPONSE:   return env.create_string("error_response");
    }
}

ReturnValue<Value> stun_method(const Env& env, const stun::Method& method) {
    if (method == stun::Method::binding()) {
        return env.create_string("binding");
    }
    return make_error_code(WrapperError::UNKNOWN_STUN_METHOD);
}

ReturnValue<Value> stun_transcation_id(const Env& env, const stun::TransactionId& id) {
    return env.create_buffer(id.view());
}

ReturnValue<Object> stun_header(const Env& env, const stun::Header& hdr) noexcept {
    return
        env.create_object({
                { "class", stun_class(env, hdr.cls) },
                { "method", stun_method(env, hdr.method) },
                { "transaction", stun_transcation_id(env, hdr.transaction_id) }
            });
}

}
