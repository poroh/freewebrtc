//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN to node.js type
//

#include "node_stun_message.hpp"
#include "node_stun_header.hpp"
#include "util/util_result_sugar.hpp"
#include "util/util_maybe.hpp"

namespace freewebrtc::node_stun {

using Value = craftnapi::Value;
using Object = craftnapi::Object;
using Env = craftnapi::Env;
using CallbackInfo = craftnapi::CallbackInfo;

Result<Object> stun_attributes(const Env& env, const stun::TransactionId& tid, const stun::AttributeSet& attrs) {
    using MaybeRVV = Maybe<Result<Value>>;
    return
        env.create_object({
            { "username",
                    attrs.username().fmap([&](auto&& v) {
                        return env.create_string(v.get().value);
                    })},
            { "software",
                    attrs.software().fmap([&](const auto& v) {
                        return env.create_string(v.get());
                    })},
            { "xor_mapped",
                    attrs.xor_mapped().fmap([&](const auto& v) {
                        return
                            v.get().addr.to_address(tid)
                            .to_string()
                            > [&](const auto& addr) {
                                return
                                    env.create_object({
                                            { "addr", env.create_string(addr) },
                                            { "port", env.create_int32(v.get().port.value()) }
                                        });
                            };
                    })},
            { "priority",
                    attrs.priority().fmap([&](const auto& v) {
                        return env.create_uint32(v.get());
                    })},
            { "ice-controlling",
                    attrs.ice_controlling().fmap([&](const auto& v) {
                        return env.create_bigint_uint64(v.get());
                    })},
            { "ice-controlled",
                    attrs.ice_controlled().fmap([&](const auto& v) {
                        return env.create_bigint_uint64(v.get());
                    })},
            { "use-candidate", attrs.has_use_candidate() ? MaybeRVV(env.create_boolean(true)) : None{} },
            { "error_code",
                    attrs.error_code().fmap([&](const auto& v) {
                        return
                            env.create_object({
                                    { "code", env.create_int32(v.get().code)},
                                    { "reason", env.create_string(v.get().reason_phrase.value_or("")) }
                                });
                    })}
        });
}

Result<Object> message(const Env& env, const stun::Message& msg) {
    return
        env.create_object({
                { "header", header(env, msg.header) },
                { "is_rfc3489", env.create_boolean(msg.is_rfc3489) },
                { "attributes", stun_attributes(env, msg.header.transaction_id, msg.attribute_set) }
            })
        .add_context("stun message");
}

Result<Value> message_parse(Env& env, const CallbackInfo& ci) {
    return ci[0]
        > [](const auto& arg) { return arg.as_buffer(); }
        > [](const auto& view) {
            freewebrtc::stun::ParseStat parsestat;
            return freewebrtc::stun::Message::parse(view, parsestat);
        }
        > [&](const auto& msg) { return message(env, msg); }
        > Object::fmap_to_value;
}

}
