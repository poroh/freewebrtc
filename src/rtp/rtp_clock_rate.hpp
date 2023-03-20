//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Clock
//

#pragma once

#include <chrono>
#include <variant>

namespace freewebrtc::rtp {

class ClockRate {
public:
    using ValueType = unsigned;

    explicit ClockRate(ValueType);
    ValueType count() const noexcept;
private:
    unsigned m_value = 0;
};

//
// inlines
//
inline ClockRate::ClockRate(ValueType v)
    : m_value(v)
{}

inline ClockRate::ValueType ClockRate::count() const noexcept {
    return m_value;
}

}
