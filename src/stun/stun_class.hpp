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

class Class {
public:
    enum Value {
        REQUEST,
        INDICATION,
        SUCCESS_RESPONSE,
        ERROR_RESPONSE
    };
    static Class from_msg_type(uint16_t);

    Value value() const noexcept;

private:
    explicit Class(Value);
    Value m_value;
};

//
// inlines
//
inline Class::Class(Value v)
    : m_value(v)
{}

inline Class::Value Class::value() const noexcept {
    return m_value;
}

inline Class Class::from_msg_type(uint16_t v) {
    //   0                 1
    //   2  3  4 5 6 7 8 9 0 1 2 3 4 5
    //  +--+--+-+-+-+-+-+-+-+-+-+-+-+-+
    //  |M |M |M|M|M|C|M|M|M|C|M|M|M|M|
    //  |11|10|9|8|7|1|6|5|4|0|3|2|1|0|
    //  +--+--+-+-+-+-+-+-+-+-+-+-+-+-+
    const auto c0 = (v >> 4) & 1;
    const auto c1 = (v >> 8) & 1;
    switch (c0 | c1) {
        case 0: return Class(REQUEST);
        case 1: return Class(INDICATION);
        case 2: return Class(SUCCESS_RESPONSE);
        case 3: return Class(ERROR_RESPONSE);
    }
    return Class(ERROR_RESPONSE);
}

}
