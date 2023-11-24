// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP client to NAPI
//

#include <memory>

#include "stun/stun_client_udp.hpp"

#include "node_stun_client_udp.hpp"
#include "node_stun_client_udp_settings.hpp"

namespace freewebrtc::node_stun {

namespace {

ReturnValue<napi::Value> constructor(napi::Env&, const napi::CallbackInfo& info) {
    auto settings_rv = info[0]
        .fmap([](auto&& val) { return val.as_object(); })
        .fmap(client_udp_settings_from_napi);

    return
        combine([](auto&& obj, auto&& settings) {
                return obj.wrap(std::make_unique<stun::ClientUDP>(settings));
            },
            info.this_arg.as_object(),
            settings_rv);
}

}

ReturnValue<napi::Value> client_udp_class(napi::Env& env, std::string_view name) {
    return
        env.create_class(
            name,
            constructor,
            {
            });
}

}


