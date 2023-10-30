//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Constans
//

#pragma once

#include <cstdint>
#include <cstddef>

namespace freewebrtc::stun::details {

// The magic cookie field MUST contain the fixed value 0x2112A442 in
// network byte order.
static constexpr uint32_t MAGIC_COOKIE = 0x2112A442;

static constexpr size_t TRANSACTION_ID_SIZE = 96 / 8;

static constexpr size_t HEADER_SIZE = 20;

// RFC5389: All STUN messages MUST start with a 20-byte header followed by zero
// or more Attributes.
static constexpr size_t STUN_HEADER_SIZE = 20;
// Size of attribute header
static constexpr size_t STUN_ATTR_HEADER_SIZE = 4;

}
