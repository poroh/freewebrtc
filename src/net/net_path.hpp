//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Network path (from source to target)
//

#pragma once

#include "net/ip/ip_address.hpp"

namespace freewebrtc::net {

struct Path {
    ip::Address source;
    ip::Address target;
    bool operator==(const Path&) const noexcept = default;
};

}


