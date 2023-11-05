//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN to node.js type
//

#include "napi_stun_message.hpp"
#include "napi_stun_header.hpp"
#include "util/util_fmap.hpp"

namespace freewebrtc::napi {

ReturnValue<Value> stun_attributes(const Env& env, const stun::TransactionId& tid, const stun::AttributeSet& attrs) {
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
                        util::fmap(attrs.xor_mapped(), [&](const auto& v) -> ReturnValue<Value> {
                            auto addr_rvv = v.get().addr.to_address(tid).to_string();
                            if (addr_rvv.is_error()) {
                                return addr_rvv.assert_error();
                            }
                            return
                                env.create_object({
                                        { "addr", env.create_string(addr_rvv.assert_value()) },
                                        { "port", env.create_int32(v.get().port.value()) }
                                    })
                                .fmap([](const Object& obj) -> Value { return obj.to_value(); });
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
                                    })
                                .fmap([](const Object& obj) -> Value { return obj.to_value(); });
                        })}
            })
        .fmap([](const Object& obj) -> Value { return obj.to_value(); });
}

ReturnValue<Value> stun_message(const Env& env, const stun::Message& msg) {
    return
        env.create_object({
                { "header", stun_header(env, msg.header) },
                { "is_rfc3489", env.create_boolean(msg.is_rfc3489) },
                { "attributes", stun_attributes(env, msg.header.transaction_id, msg.attribute_set) }
            })
        .fmap([](const Object& obj) -> Value { return obj.to_value(); });
}

ReturnValue<Value> stun_message_parse(Env& env, const CallbackInfo& ci) {
    if (ci.args.size() == 0) {
        return env.throw_error("First argument must be a buffer");
    }

    auto as_buffer_result = ci.args[0].as_buffer();
    if (as_buffer_result.is_error()) {
        return as_buffer_result.assert_error();
    }

    const auto& view = as_buffer_result.assert_value();

    freewebrtc::stun::ParseStat parsestat;
    auto maybe_msg = freewebrtc::stun::Message::parse(view, parsestat);
    if (!maybe_msg.has_value()) {
        return env.throw_error("Failed to parse STUN packet");
    }
    return stun_message(env, maybe_msg.value());
}

}
