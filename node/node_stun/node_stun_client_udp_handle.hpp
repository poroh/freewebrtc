//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP client handle to NAPI
//

#pragma once

#include <string_view>
#include <craftnapi/env.hpp>

namespace freewebrtc::node_stun {

Result<craftnapi::Value> client_udp_handle_to_craftnapi(craftnapi::Env& env, stun::ClientUDP::Handle hnd);


//
// inlines
//
inline Result<craftnapi::Value> client_udp_handle_to_craftnapi(craftnapi::Env& env, stun::ClientUDP::Handle hnd) {
    return env.create_object({
            {"value", env.create_int32(hnd.value)}
        })
        .fmap(craftnapi::Object::fmap_to_value);
}


}
