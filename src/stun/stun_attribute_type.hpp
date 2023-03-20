//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Attribute Type
//

#pragma once

#include <cstdint>
#include <functional>

namespace freewebrtc::stun {

class AttributeType {
public:
    static AttributeType from_uint16(uint16_t);

    bool operator==(const AttributeType&) const noexcept;

    uint16_t value() const noexcept;
    bool is_comprehension_required() const noexcept;

private:
    explicit AttributeType(uint16_t v);
    uint16_t m_value;
};

//
// inlines
//
inline AttributeType::AttributeType(uint16_t v)
    : m_value(v)
{}

inline AttributeType AttributeType::from_uint16(uint16_t v) {
    return AttributeType(v);
}

inline bool AttributeType::operator==(const AttributeType& other) const noexcept {
    return m_value == other.m_value;
}

inline uint16_t AttributeType::value() const noexcept {
    return m_value;
}

inline bool AttributeType::is_comprehension_required() const noexcept {
    // STUN attribute types in the range 0x0000 - 0x7FFF are considered
    // comprehension-required;
    return m_value <= 0x7FFF;
}

}

namespace std {
    template<>
    struct hash<freewebrtc::stun::AttributeType> {
        size_t operator()(const freewebrtc::stun::AttributeType& type) const {
            return hash<uint16_t>()(type.value());
        }
    };
}
