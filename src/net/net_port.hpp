//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Network port (TCP / UDP / SCTP / ...)
//

#pragma once

#include <cstdint>

namespace freewebrtc::net {

class Port {
public:
    explicit Port(uint16_t);

    uint16_t value() const noexcept;
private:
    uint16_t m_value;
};

//
// implementation
//
inline Port::Port(uint16_t p)
    : m_value(p)
{}

inline uint16_t Port::value() const noexcept {
    return m_value;
}

}
