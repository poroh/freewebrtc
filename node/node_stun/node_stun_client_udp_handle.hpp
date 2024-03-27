//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP client handle to NAPI
//

#pragma once

#include <string_view>
#include "node/napi_wrapper/napi_wrapper.hpp"

namespace freewebrtc::node_stun {

Result<napi::Value> client_udp_handle_to_napi(napi::Env& env, stun::ClientUDP::Handle hnd);


//
// inlines
//
inline Result<napi::Value> client_udp_handle_to_napi(napi::Env& env, stun::ClientUDP::Handle hnd) {
    return env.create_object({
            {"value", env.create_int32(hnd.value)}
        })
        .fmap(napi::Object::fmap_to_value);
}


}
