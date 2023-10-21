//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN to node.js type
//

#include "napi_stun_message.hpp"
#include "napi_stun_header.hpp"
#include <iostream>

namespace freewebrtc::napi {

ReturnValue<Value> stun_message(const Env& env, const stun::Message& msg) {
    return
        env.create_object({
                { "header", stun_header(env, msg.header) }
            })
        .fmap([](const Object& obj) -> Value { return obj.to_value(); }); // transform from Object to Value
}

}
