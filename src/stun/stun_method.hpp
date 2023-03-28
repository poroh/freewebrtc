//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Method
//

#pragma once

#include <cstdint>

namespace freewebrtc::stun {

class Method {
public:
    static Method from_msg_type(uint16_t) noexcept;
    static Method binding() noexcept;

    unsigned value() const;
    bool operator==(const Method&) const noexcept = default;
private:
    explicit Method(unsigned);
    unsigned m_value;
};

//
// inlines
//
inline Method::Method(unsigned v)
    : m_value(v)
{}

inline Method Method::from_msg_type(uint16_t v) noexcept {
    //   0                 1
    //   2  3  4 5 6 7 8 9 0 1 2 3 4 5
    //  +--+--+-+-+-+-+-+-+-+-+-+-+-+-+
    //  |M |M |M|M|M|C|M|M|M|C|M|M|M|M|
    //  |11|10|9|8|7|1|6|5|4|0|3|2|1|0|
    //  +--+--+-+-+-+-+-+-+-+-+-+-+-+-+
    const auto m0_3  = v & 0xF;
    const auto m4_6  = (v >> 5) & 0x7;
    const auto m7_11 = (v >> 9);
    return Method(m0_3 | (m4_6 << 4) | (m7_11 << 7));
}

}


