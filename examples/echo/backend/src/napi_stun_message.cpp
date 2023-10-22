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
                            if (addr_rvv.error().has_value()) {
                                return addr_rvv.error().value();
                            }
                            return
                                env.create_object({
                                        { "addr", env.create_string(addr_rvv.value().value().get()) },
                                        { "port", env.create_int32(v.get().port.value()) }
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

}
