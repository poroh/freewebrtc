//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Data structures
//

#pragma once

#include <cstdint>

namespace freewebrtc::rtp {

class SSRC {
public:
    static SSRC from_uint32(uint32_t);
private:
    uint32_t m_value;
};

}
