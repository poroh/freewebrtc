//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Sequence number
//

#pragma once

#include <cstdint>

namespace freewebrtc::rtp {

class SequenceNumber {
public:
    static SequenceNumber from_uint16(uint16_t);
private:
    uint16_t m_value;
};

}
