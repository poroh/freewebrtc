// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP client to NAPI
//

#include <memory>
#include <iostream>

#include "util/util_fmap.hpp"
#include "util/util_return_value_sugar.hpp"
#include "stun/stun_client_udp.hpp"
#include "stun/details/stun_client_udp_rto.hpp"
#include "clock/clock_std.hpp"

#include "node_stun_client_udp.hpp"
#include "node_stun_client_udp_settings.hpp"
#include "node_stun_client_udp_effects.hpp"
#include "node_stun_client_udp_handle.hpp"
#include "node_stun_client_udp_auth.hpp"

namespace freewebrtc::node_stun {

namespace {

ReturnValue<napi::Value> constructor(napi::Env&, const napi::CallbackInfo& info) {
    auto settings_rv = info[0] > napi::Value::to_object > client_udp_settings_from_napi;
    return
        combine([](auto&& obj, auto&& settings) {
                return obj.wrap(std::make_unique<stun::ClientUDP>(settings));
            },
            info.this_arg.as_object(),
            settings_rv);
}

ReturnValue<stun::ClientUDP::Request> request_from_napi(const napi::Object& obj) {
    auto source_rv = (obj.named_property("source")
        > napi::Value::to_string
        > net::ip::Address::from_string
        ).add_context("source field");
    auto target_rv = (obj.named_property("target")
        > napi::Value::to_string
        > net::ip::Address::from_string
        ).add_context("target field");

    using Auth = stun::ClientUDP::Auth;
    using MaybeAuth = stun::ClientUDP::MaybeAuth;

    ReturnValue<MaybeAuth> maybe_auth_rv
        = (obj.maybe_named_property("auth")
           >= [](auto&& maybe_val) { return util::fmap(maybe_val, napi::Value::to_object); }
           >  [](auto&& maybe_obj) {
               if (!maybe_obj.has_value()) {
                   return ReturnValue<MaybeAuth>{std::nullopt};
               }
               return maybe_obj.value() > parse_auth >= [](Auth&& auth) { return MaybeAuth{std::move(auth)}; };
           }).add_context("auth field");

    return combine(
        [](net::ip::Address&& src, net::ip::Address&& dst, MaybeAuth&& maybe_auth) -> ReturnValue<stun::ClientUDP::Request> {
            return stun::ClientUDP::Request{
                .path = {
                    .source = std::move(src),
                    .target  =std::move(dst)
                },
                .maybe_auth = maybe_auth
            };
        }
        , std::move(source_rv)
        , std::move(target_rv)
        , std::move(maybe_auth_rv))
        .add_context("request");
}

ReturnValue<napi::Value> create(napi::Env& env, const napi::CallbackInfo& info) {
    auto request_rv = info[0] > napi::Value::to_object > request_from_napi;
    auto client_rv = info.this_arg.unwrap<stun::ClientUDP>();
    return combine(
        [](auto&& client, stun::ClientUDP::Request&& req) {
            return client.get().create(random, clock::steady_clock_now(), std::move(req));
        }
        , std::move(client_rv)
        , std::move(request_rv))
        .bind([&](auto&& hnd) { return client_udp_handle_to_napi(env, hnd); })
        .add_context("create request");
}

ReturnValue<napi::Value> next(napi::Env& env, const napi::CallbackInfo& info) {
    return info.this_arg.unwrap<stun::ClientUDP>()
        > [&](auto&& client) {
            auto effect = client.get().next(clock::steady_clock_now());
            return client_udp_effect_to_napi(env, effect);
        }
        >= napi::Object::fmap_to_value;
}

ReturnValue<napi::Value> response(napi::Env& env, const napi::CallbackInfo& info) {
    auto this_rv = info.this_arg.unwrap<stun::ClientUDP>();
    auto resp_rv = info[0] > napi::Value::to_buffer;
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


