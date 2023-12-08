//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP client settings to NAPI
//

#include "node_stun_client_udp_settings.hpp"
#include "util/util_return_value_sugar.hpp"

namespace freewebrtc::node_stun {

namespace {

using MaybeBool = std::optional<bool>;
using Settings = stun::client_udp::Settings;

}

ReturnValue<stun::client_udp::Settings> client_udp_settings_from_napi(napi::Object obj) {
    ReturnValue<MaybeBool> maybe_use_fingerprint_rv
        = (obj.maybe_named_property("use_fingerprint")
           > [](auto&& maybe_v) {
               if (!maybe_v.has_value()) {
                   return ReturnValue<MaybeBool>{std::nullopt};
               }
               return maybe_v.value().as_boolean() >= [](bool v) { return MaybeBool{v}; };
           }).add_context("use_fingerprint attibute");

    return combine(
        [](auto&& maybe_use_fingerprint) -> ReturnValue<stun::client_udp::Settings> {
            stun::client_udp::Settings result;
            if (maybe_use_fingerprint.has_value()) {
                result.use_fingerprint = Settings::UseFingerprint{maybe_use_fingerprint.value()};
            }
            return result;
        }
        , std::move(maybe_use_fingerprint_rv))
        .add_context("stun udp client settings");
}

}
