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

    static Class from_msg_type(uint16_t) noexcept;

    static Class request() noexcept;
    static Class indication() noexcept;
    static Class success_response() noexcept;
    static Class error_response() noexcept;

    uint16_t to_msg_type() const noexcept;
    Value value() const noexcept;

    bool operator==(const Class&) const = default;
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

inline Class Class::from_msg_type(uint16_t v) noexcept {
    //   0                 1
    //   2  3  4 5 6 7 8 9 0 1 2 3 4 5
    //  +--+--+-+-+-+-+-+-+-+-+-+-+-+-+
    //  |M |M |M|M|M|C|M|M|M|C|M|M|M|M|
    //  |11|10|9|8|7|1|6|5|4|0|3|2|1|0|
    //  +--+--+-+-+-+-+-+-+-+-+-+-+-+-+
    const auto c0 = (v >> 4) & 1;
    const auto c1 = (v >> 8) & 1;
    switch (c0 | (c1 << 1)) {
        case 0: return Class(REQUEST);
        case 1: return Class(INDICATION);
        case 2: return Class(SUCCESS_RESPONSE);
        case 3: return Class(ERROR_RESPONSE);
    }
    return Class(ERROR_RESPONSE);
}

inline uint16_t Class::to_msg_type() const noexcept {
    return ((m_value & 0x1) << 4)
        | ((m_value >> 1) << 8);
}


inline Class Class::request() noexcept {
    return Class(REQUEST);
}

inline Class Class::indication() noexcept  {
    return Class(INDICATION);
}

inline Class Class::success_response() noexcept {
    return Class(SUCCESS_RESPONSE);
}

inline Class Class::error_response() noexcept {
    return Class(ERROR_RESPONSE);
}

}
