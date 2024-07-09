// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP Client Auth bindings
//

#include "node/node_stun/node_stun_client_udp_auth.hpp"
#include "node/openssl/node_openssl_hash.hpp"
#include "util/util_result_sugar.hpp"

namespace freewebrtc::node_stun {

Result<stun::ClientUDP::Auth> parse_auth(craftnapi::Object obj) {
    const auto sha1 = freewebrtc::crypto::node_openssl::sha1;
    auto username_rv = (obj.named_property("username")
        > [](auto&& val) { return val.as_string(); }
        > [](std::string&& str) { return precis::OpaqueString{std::move(str)}; }
        ).add_context("username attribute");

    auto password_rv = (obj.named_property("password")
        > [](auto&& val) { return val.as_string(); }
        > [&](std::string&& str) {
            return stun::Password::short_term(precis::OpaqueString{std::move(str)}, sha1);
        }
        ).add_context("password attribute");


    return combine(
        [&](auto&& username, auto&& password) -> Result<stun::ClientUDP::Auth> {
            return stun::ClientUDP::Auth{username, stun::IntegrityData{password, sha1}};
        }
        , std::move(username_rv)
        , std::move(password_rv))
        .add_context("authenticaiton");

}

}
