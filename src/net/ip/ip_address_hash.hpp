//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Hash for IP address to use it in unordered_map / unordered_set
//

#pragma once

#include <cstddef>

#include "net/ip/ip_address.hpp"

namespace freewebrtc::net::ip {

struct AddressHash {
    std::size_t operator()(const net::ip::Address&);
};


}
