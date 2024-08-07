//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of ICE candidate functions to NAPI
//

#include "node_ice_candidate.hpp"
#include "util/util_result_sugar.hpp"
#include "ice/candidate/ice_candidate_sdp.hpp"

namespace freewebrtc::node_ice {

Result<craftnapi::Object> ice_candidate_to_object(craftnapi::Env& env, const ice::candidate::Supported& v) {
    const auto& c = v.candidate;
    auto ice_addr_to_string = [&](auto&& addr) { return addr.to_string() > [&](auto&& str) { return env.create_string(str); }; };
    auto ext_to_obj = [&](auto&& ext) {
        return env.create_object({
            {"name", env.create_string(ext.att_name)},
            {"value", env.create_string(ext.att_value)}})
            > craftnapi::Object::fmap_to_value;
    };
    return env.create_object({
        { "result", env.create_object({
                { "host", ice_addr_to_string(c.address) },
                { "port", env.create_int32(c.port.value()) },
                { "type", env.create_string(c.type.to_string()) },
                { "transport", env.create_string(c.transport_type.to_string()) },
                { "foundation", env.create_string(c.foundation.to_string()) },
                { "component", env.create_int32(c.component.value()) },
                { "raddr", c.maybe_related_address.fmap(ice_addr_to_string) },
                { "rport", c.maybe_related_port.fmap([&](auto&& rport) { return env.create_int32(rport.value()); }) },
                { "extensions", env.create_array(v.extensions, ext_to_obj) }}) },
        { "error", env.create_null() }
    });
}

Result<craftnapi::Value> ice_candidate_parse(craftnapi::Env& env, const craftnapi::CallbackInfo& ci) {
    return ci[0]
        > craftnapi::Value::to_string
        > ice::candidate::parse_sdp_attr
        > [&](auto&& c) { return std::visit(
            [&](auto&& result) {
                if constexpr (std::is_same_v<ice::candidate::Supported&&, decltype(result)>) {
                    return ice_candidate_to_object(env, result)
                        > craftnapi::Object::fmap_to_value;
                } else if constexpr (std::is_same_v<ice::candidate::Unsupported&&, decltype(result)>) {
                    return env.create_object({
                        { "result", env.create_null() },
                        { "error", env.create_string(result.value) }})
                        > craftnapi::Object::fmap_to_value;
                }
            },
            std::move(c));
        };
}

}
