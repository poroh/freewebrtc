//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP client effects to NAPI
//

#include <iostream>

#include "util/util_result_sugar.hpp"
#include "stun/stun_client_udp.hpp"
#include "node_stun_client_udp_effects.hpp"
#include "node_stun_client_udp_handle.hpp"
#include "node_stun_message.hpp"

namespace freewebrtc::node_stun {

namespace {

Result<craftnapi::Object> send_data_to_craftnapi(craftnapi::Env& env, const stun::ClientUDP::SendData& d) {
    return env.create_object({
        {"type", env.create_string("send_data")},
        {"message", env.create_buffer(d.message_view)}
    });
}

Result<craftnapi::Object> transaction_ok_to_craftnapi(craftnapi::Env& env, const stun::ClientUDP::TransactionOk& t) {
    return env.create_object({
        {"type", env.create_string("transaction_ok")},
        {"result", env.create_object({
                { "addr", t.result.address.to_string() > [&](auto&& s) { return env.create_string(std::move(s)); }},
                { "port", env.create_int32(t.result.port.value()) }
                })},
        {"response", message(env, t.response)},
        {"rtt_us", t.round_trip.fmap([&](auto rtt) {
            return env.create_int32(std::chrono::duration_cast<std::chrono::microseconds>(rtt).count());
        })}
    });
}


Result<craftnapi::Value> transaction_failed_reason_to_craftnapi(craftnapi::Env& env, const stun::ClientUDP::TransactionFailed::Reason& reason) {
    return std::visit(
        util::overloaded {
            [&](const stun::ClientUDP::TransactionFailed::UnknownComprehensionRequiredAttribute&) {
                return env.create_string("unknown comprehension required attribute");
            },
            [&](const stun::ClientUDP::TransactionFailed::UnknownAttributeReported&){
                return env.create_string("unknown required attribute reported");
            },
            [&](const stun::ClientUDP::TransactionFailed::AlternateServer&){
                return env.create_string("alternate server");
            },
            [&](const stun::ClientUDP::TransactionFailed::ErrorCode& ec){
                return env.create_string("stun error: " + std::to_string(ec.attr.code) + ": " + ec.attr.reason_phrase.value_or(""));
            },
            [&](const stun::ClientUDP::TransactionFailed::Error& err){
                return env.create_string("error: " + err.code.message());
            },
            [&](const stun::ClientUDP::TransactionFailed::Timeout&){
                return env.create_string("timeout");
            }},
        reason);
}

Result<craftnapi::Object> transaction_failed_to_craftnapi(craftnapi::Env& env, const stun::ClientUDP::TransactionFailed& f) {
    return env.create_object({
        {"type", env.create_string("transaction_fail")},
        {"handle", client_udp_handle_to_craftnapi(env, f.handle)},
        {"reason", transaction_failed_reason_to_craftnapi(env, f.reason)}
    });
}

Result<craftnapi::Object> sleep_to_craftnapi(craftnapi::Env& env, const stun::ClientUDP::Sleep& s) {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(s.sleep);
    auto adj = s.sleep == ms ? 0 : 1; // adjust to next whole ms
    return env.create_object({
        {"type", env.create_string("sleep")},
        {"timeout_ms", env.create_int32(ms.count() + adj)}
    });
}

}

Result<craftnapi::Object> client_udp_effect_to_craftnapi(craftnapi::Env& env, const stun::ClientUDP::Effect& effect) {
    return std::visit(
        util::overloaded {
            [&](const stun::ClientUDP::SendData& d)          { return send_data_to_craftnapi(env, d); },
            [&](const stun::ClientUDP::TransactionOk& t)     { return transaction_ok_to_craftnapi(env, t); },
            [&](const stun::ClientUDP::TransactionFailed& f) { return transaction_failed_to_craftnapi(env, f); },
            [&](const stun::ClientUDP::Sleep& s)             { return sleep_to_craftnapi(env, s); },
            [&](const stun::ClientUDP::Idle&)                { return env.create_object({{"type", env.create_string("idle")}}); }
            }
        , effect);
}

}
