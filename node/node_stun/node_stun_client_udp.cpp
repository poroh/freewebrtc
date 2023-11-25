// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP client to NAPI
//

#include <memory>
#include <iostream>

#include "stun/stun_client_udp.hpp"
#include "clock/clock_std.hpp"

#include "node_stun_client_udp.hpp"
#include "node_stun_client_udp_settings.hpp"
#include "node_stun_client_udp_effects.hpp"
#include "node_stun_client_udp_handle.hpp"

namespace freewebrtc::node_stun {

namespace {

ReturnValue<napi::Value> constructor(napi::Env&, const napi::CallbackInfo& info) {
    auto settings_rv = info[0]
        .fmap(napi::Value::to_object)
        .fmap(client_udp_settings_from_napi);

    return
        combine([](auto&& obj, auto&& settings) {
                return obj.wrap(std::make_unique<stun::ClientUDP>(settings));
            },
            info.this_arg.as_object(),
            settings_rv);
}

ReturnValue<stun::ClientUDP::Request> request_from_napi(const napi::Object& obj) {
    auto target_rv = obj.named_property("target")
        .fmap(napi::Value::to_object)
        .fmap([](auto&& obj) {
            auto addr_rv = obj.named_property("addr")
                .fmap(napi::Value::to_string)
                .fmap(net::ip::Address::from_string);
            auto port_rv = obj.named_property("port")
                .fmap(napi::Value::to_int32)
                .fmap([](auto value) -> ReturnValue<uint16_t> {
                    if (value > 0xFFFF || value < 0) {
                        return make_error_code(napi::WrapperError::INVALID_PORT_NUMBER);
                    }
                    return (uint16_t)value;
                });
            return combine(
                [](net::ip::Address&& addr, auto&& port) {
                    return net::UdpEndpoint{std::move(addr), net::Port{port}};
                }
                , std::move(addr_rv)
                , std::move(port_rv));
        });

    return combine(
        [](net::UdpEndpoint&& ep) {
            return stun::ClientUDP::Request{ep};
        }
        , std::move(target_rv));
}

ReturnValue<napi::Value> create(napi::Env& env, const napi::CallbackInfo& info) {
    auto request_rv = info[0]
        .fmap(napi::Value::to_object)
        .fmap(request_from_napi);
    auto client_rv = info.this_arg.unwrap<stun::ClientUDP>();
    return combine(
        [](auto&& client, stun::ClientUDP::Request&& req) {
            return client.get().create(random, clock::steady_clock_now(), std::move(req));
        }
        , std::move(client_rv)
        , std::move(request_rv))
        .fmap([&](auto&& hnd) {
            return client_udp_handle_to_napi(env, hnd);
        });
}

ReturnValue<napi::Value> next(napi::Env& env, const napi::CallbackInfo& info) {
    return info.this_arg.unwrap<stun::ClientUDP>()
        .fmap([&](auto&& client) {
            auto effect = client.get().next(clock::steady_clock_now());
            return client_udp_effect_to_napi(env, effect);
        })
        .fmap(napi::Object::fmap_to_value);
}

ReturnValue<napi::Value> response(napi::Env& env, const napi::CallbackInfo& info) {
    auto this_rv = info.this_arg.unwrap<stun::ClientUDP>();
    auto resp_rv = info[0].fmap(napi::Value::to_buffer);
    return combine(
        [&](auto& client, auto& resp) {
            client.get().response(clock::steady_clock_now(), resp);
            return env.create_undefined();
        },
        this_rv,
        resp_rv);
}

}

ReturnValue<napi::Value> client_udp_class(napi::Env& env, std::string_view name) {
    return
        env.create_class(
            name,
            constructor,
            {
                {"create",  create},
                {"next",    next},
                {"response",response}
            });
}

}


