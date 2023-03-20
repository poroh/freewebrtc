//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Attributes Registry
//

#pragma once

#include <cstdint>

namespace freewebrtc::stun::attr_registry {
// RFC5389:
// 18.2.  STUN Attribute Registry / Comprehesion required
static constexpr uint16_t MAPPED_ADDRESS     = 0x0001;
static constexpr uint16_t USERNAME           = 0x0006;
static constexpr uint16_t MESSAGE_INTEGRITY  = 0x0008;
static constexpr uint16_t ERROR_CODE         = 0x0009;
static constexpr uint16_t REALM              = 0x0014;
static constexpr uint16_t NONCE              = 0x0016;
static constexpr uint16_t XOR_MAPPED_ADDRESS = 0x0020;

// Comprehension-optional
static constexpr uint16_t SOFTWARE         = 0x8022;
static constexpr uint16_t ALTERNATE_SERVER = 0x8023;
static constexpr uint16_t FINGERPRINT      = 0x8028;

}

