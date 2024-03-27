//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Address
//

#include "ice/candidate/ice_candidate_address.hpp"
#include "ice/candidate/ice_candidate_error.hpp"

namespace freewebrtc::ice::candidate {

Result<Address> Address::from_string(std::string_view v) noexcept {
    return net::ip::Address::from_string(v)
        .fmap([](net::ip::Address&& ip) {
            return Address{std::move(ip)};
        })
        .bind_err([&](auto&& iperr) {
            return net::Fqdn::from_string(v)
                .fmap([](net::Fqdn&& fqdn) {
                    return Address{std::move(fqdn)};
                })
                .bind_err([&](auto&&) {
                    return iperr;
                });
        });
}

}
