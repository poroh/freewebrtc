//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP client settings to NAPI
//

#include "util/util_fmap.hpp"

#include "node/openssl/node_openssl_hash.hpp"

#include "node_stun_client_udp_settings.hpp"

namespace freewebrtc::node_stun {

namespace {

using Settings = stun::client_udp::Settings;
using Auth = Settings::Auth;
using MaybeAuth = std::optional<Auth>;
using MaybeBool = std::optional<bool>;

ReturnValue<Auth> parse_auth(napi::Object obj) {
    const auto sha1 = freewebrtc::crypto::node_openssl::sha1;
    auto username_rv = obj.named_property("username")
        .fmap([](auto&& val) { return val.as_string(); })
        .fmap([](std::string&& str) { return precis::OpaqueString{std::move(str)}; });

    auto password_rv = obj.named_property("password")
        .fmap([](auto&& val) { return val.as_string(); })
        .fmap([&](std::string&& str) {
            return stun::Password::short_term(precis::OpaqueString{std::move(str)}, sha1);
         });

    return combine(
        [&](auto&& username, auto&& password) {
            return stun::client_udp::Settings::Auth{username, stun::IntegrityData{password, sha1}};
        }
        , std::move(username_rv)
        , std::move(password_rv));
}

}

ReturnValue<stun::client_udp::Settings> client_udp_settings_from_napi(napi::Object obj) {
    ReturnValue<MaybeAuth> maybe_auth_rv
        = obj.maybe_named_property("auth")
        .fmap([](auto&& maybe_val) { return util::fmap(maybe_val, napi::Value::to_object); })
        .fmap([](auto&& maybe_obj) {
            if (!maybe_obj.has_value()) {
                return ReturnValue<MaybeAuth>{std::nullopt};
            }
            return maybe_obj.value()
                .fmap(parse_auth)
                .fmap([](Auth&& auth) {
                    return MaybeAuth{std::move(auth)};
                });
        });

    ReturnValue<MaybeBool> maybe_use_fingerprint_rv
        = obj.maybe_named_property("use_fingerprint")
        .fmap([](auto&& maybe_v) {
            if (!maybe_v.has_value()) {
                return ReturnValue<MaybeBool>{std::nullopt};
            }
            return maybe_v.value().as_boolean()
                .fmap([](bool v) { return ReturnValue<MaybeBool>{v}; });
        });

    return combine(
        [](MaybeAuth&& maybe_auth, auto&& maybe_use_fingerprint) {
            stun::client_udp::Settings result;
            result.maybe_auth = std::move(maybe_auth);
            if (maybe_use_fingerprint.has_value()) {
                result.use_fingerprint = Settings::UseFingerprint{maybe_use_fingerprint.value()};
            }
            return result;
        }
        , std::move(maybe_auth_rv)
        , std::move(maybe_use_fingerprint_rv));
}

}
