//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Address
//

#pragma once

#include <string_view>
#include "net/ip/ip_address.hpp"
#include "net/net_fqdn.hpp"

namespace freewebrtc::ice::candidate {

class Address {
public:
    static ReturnValue<Address> from_string(std::string_view v);

private:
    std::variant<net::ip::Address, net::Fqdn> m_value;
};

}
