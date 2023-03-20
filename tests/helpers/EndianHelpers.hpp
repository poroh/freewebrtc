//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Packet Helpers for unit tests
//

#pragma once

#include <cstdint>
#include <vector>

#include "util/UtilEndian.hpp"

namespace freewebrtc::test::helpers {

inline std::vector<uint8_t> uint32be(uint32_t v) {
    std::vector<uint8_t> result(sizeof(v));
    v = util::host_to_network_u32(v);
    memcpy(result.data(), &v, result.size());
    return result;
}

inline std::vector<uint8_t> uint16be(uint16_t v) {
    std::vector<uint8_t> result(sizeof(v));
    v = util::host_to_network_u16(v);
    memcpy(result.data(), &v, result.size());
    return result;
}

}


