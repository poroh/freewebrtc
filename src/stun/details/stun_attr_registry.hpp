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
static constexpr uint16_t UNKNOWN_ATTRIBUTES = 0x000A;
static constexpr uint16_t REALM              = 0x0014;
static constexpr uint16_t NONCE              = 0x0016;
static constexpr uint16_t XOR_MAPPED_ADDRESS = 0x0020;
// RFC8445:
// 20.1.  STUN Attributes
static constexpr uint16_t PRIORITY        = 0x24;
static constexpr uint16_t USE_CANDIDATE   = 0x25;

//
// Comprehension-optional
//
static constexpr uint16_t COMPREHANENSION_OPTIONAL = 0x8000;
static constexpr uint16_t SOFTWARE         = 0x8022;
static constexpr uint16_t ALTERNATE_SERVER = 0x8023;
static constexpr uint16_t FINGERPRINT      = 0x8028;
// RFC8445:
// 20.1.  STUN Attributes
static constexpr uint16_t ICE_CONTROLLED  = 0x8029;
static constexpr uint16_t ICE_CONTROLLING = 0x802A;

// RFC8489 14.1.  MAPPED-ADDRESS
// Address family
static constexpr uint8_t FAMILY_IPV4 = 0x01;
static constexpr uint8_t FAMILY_IPV6 = 0x02;

}

