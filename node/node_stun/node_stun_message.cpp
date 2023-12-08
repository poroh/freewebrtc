//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN to node.js type
//

#include "node_stun_message.hpp"
#include "node_stun_header.hpp"
#include "util/util_fmap.hpp"
#include "util/util_return_value_sugar.hpp"

namespace freewebrtc::node_stun {

using Value = napi::Value;
using Object = napi::Object;
using Env = napi::Env;
using CallbackInfo = napi::CallbackInfo;

ReturnValue<Object> stun_attributes(const Env& env, const stun::TransactionId& tid, const stun::AttributeSet& attrs) {
    using MaybeRVV = std::optional<ReturnValue<Value>>;
    return
        env.create_object({
            { "username",
                    util::fmap(attrs.username(), [&](const auto& v) {
                        return env.create_string(v.get().value);
                    })},
            { "software",
                    util::fmap(attrs.software(), [&](const auto& v) {
                        return env.create_string(v.get());
                    })},
            { "xor_mapped",
                    util::fmap(attrs.xor_mapped(), [&](const auto& v) {
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
                    util::fmap(attrs.priority(), [&](const auto& v) {
                        return env.create_uint32(v.get());
                    })},
            { "ice-controlling",
                    util::fmap(attrs.ice_controlling(), [&](const auto& v) {
                        return env.create_bigint_uint64(v.get());
                    })},
            { "ice-controlled",
                    util::fmap(attrs.ice_controlled(), [&](const auto& v) {
                        return env.create_bigint_uint64(v.get());
                    })},
            { "use-candidate", attrs.has_use_candidate() ? MaybeRVV(env.create_boolean(true)) : std::nullopt },
            { "error_code",
                    util::fmap(attrs.error_code(), [&](const auto& v) {
                        return
                            env.create_object({
                                    { "code", env.create_int32(v.get().code)},
                                    { "reason", env.create_string(v.get().reason_phrase.value_or("")) }
                                });
                    })}
        });
}

ReturnValue<Object> message(const Env& env, const stun::Message& msg) {
    return
        env.create_object({
                { "header", header(env, msg.header) },
                { "is_rfc3489", env.create_boolean(msg.is_rfc3489) },
                { "attributes", stun_attributes(env, msg.header.transaction_id, msg.attribute_set) }
            })
        .add_context("stun message");
}

ReturnValue<Value> message_parse(Env& env, const CallbackInfo& ci) {
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
