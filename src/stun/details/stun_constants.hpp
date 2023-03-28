//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Constans
//

#pragma once

#include <cstdint>

namespace freewebrtc::stun::details {

// The magic cookie field MUST contain the fixed value 0x2112A442 in
// network byte order.
static constexpr uint32_t MAGIC_COOKIE = 0x2112A442;

}
