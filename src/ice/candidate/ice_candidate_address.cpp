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
    auto ipaddr_rv = net::ip::Address::from_string(v);
    if (ipaddr_rv.is_ok()) {
        return Address{std::move(ipaddr_rv.unwrap())};
    }
    auto fqdn_rv = net::Fqdn::from_string(v);
    if (fqdn_rv.is_ok()) {
        return Address(std::move(fqdn_rv.unwrap()));
    }
    return ipaddr_rv.unwrap_err();
}

}
