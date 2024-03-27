//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Network port (TCP / UDP / SCTP / ...)
//

#pragma once

#include <cstdint>
#include "util/util_result.hpp"

namespace freewebrtc::net {

class Port {
public:
    explicit Port(uint16_t);
    static Port from_uint16(uint16_t) noexcept;
    static Result<Port> from_string(const std::string_view&) noexcept;

    bool operator==(const Port&) const noexcept = default;
    uint16_t value() const noexcept;
private:
    uint16_t m_value;
};

//
// implementation
//
inline Port::Port(uint16_t v)
    : m_value(v)
{}

inline Port Port::from_uint16(uint16_t v) noexcept {
    return Port(v);
}

inline uint16_t Port::value() const noexcept {
    return m_value;
}

}
