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
    uint16_t value() const noexcept;
private:
    explicit SequenceNumber(uint16_t);
    uint16_t m_value;
};


//
// inlines
//
inline SequenceNumber::SequenceNumber(uint16_t v)
    : m_value(v)
{}

inline SequenceNumber SequenceNumber::from_uint16(uint16_t v) {
    return SequenceNumber{v};
}

inline uint16_t SequenceNumber::value() const noexcept {
    return m_value;
}

}
