//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Fingerprint calculation
//

#pragma once

#include <cstdint>

namespace freewebrtc::util {
class ConstBinaryView;
}

namespace freewebrtc::stun {

uint32_t crc32(const util::ConstBinaryView& vv);

static constexpr uint32_t FINGERPRINT_XOR = 0x5354554e;

}
