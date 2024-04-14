//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP client settings to NAPI
//

#include "node_stun_client_udp_settings.hpp"
#include "util/util_result_sugar.hpp"

namespace freewebrtc::node_stun {

namespace {

using MaybeBool = Maybe<bool>;
using Settings = stun::client_udp::Settings;

}

Result<stun::client_udp::Settings> client_udp_settings_from_napi(napi::Object obj) {
    Result<MaybeBool> maybe_use_fingerprint_rv
        = (obj.maybe_named_property("use_fingerprint")
           > [](auto&& maybe_v) {
               return maybe_v
                   .fmap([](auto&& v) {
                       return v.as_boolean()
                           > [](const bool& v) { return MaybeBool{v}; };
                   })
                   .value_or(Result<MaybeBool>{none()});
           }).add_context("use_fingerprint attibute");

    return combine(
        [](auto&& maybe_use_fingerprint) -> Result<stun::client_udp::Settings> {
            stun::client_udp::Settings result;
            result.use_fingerprint = maybe_use_fingerprint
                .fmap([](auto&& v) {
                    return Settings::UseFingerprint{v};
                })
                .value_or(result.use_fingerprint);
            return result;
        }
        , std::move(maybe_use_fingerprint_rv))
        .add_context("stun udp client settings");
}

}
