//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Timestamp
//

#pragma once

#include <cstdint>
#include "rtp/RTPClockRate.hpp"

namespace freewebrtc::rtp {

class Timestamp {
    using ValueType = uint32_t;
public:
    static Timestamp from_uint32(ValueType, ClockRate clock);
    ValueType value() const noexcept;

private:
    Timestamp(ValueType v, ClockRate c);
    ValueType m_value;
    ClockRate m_rate;
};

//
// inlines
//
inline Timestamp::Timestamp(ValueType v, ClockRate cr)
    : m_value(v)
    , m_rate(cr)
{}

inline Timestamp Timestamp::from_uint32(ValueType v, ClockRate c) {
    return Timestamp(v, c);
}

inline Timestamp::ValueType Timestamp::value() const noexcept {
    return m_value;
}


}
