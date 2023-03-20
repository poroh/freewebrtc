//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Payload Type
//

#pragma once

#include <cstdint>
#include <optional>

namespace freewebrtc::rtp {

class PayloadType {
public:
    uint8_t value() const noexcept;
    static std::optional<PayloadType> from_uint8(uint8_t pt);
    bool operator==(const PayloadType&) const noexcept;

private:
    explicit PayloadType(unsigned value);
    unsigned m_value;
};

//
// inlines
//
inline PayloadType::PayloadType(unsigned v)
    : m_value(v)
{}

inline uint8_t PayloadType::value() const noexcept {
    return m_value;
}

inline bool PayloadType::operator==(const PayloadType& other) const noexcept {
    return m_value == other.m_value;
}

inline std::optional<PayloadType> PayloadType::from_uint8(uint8_t pt) {
    if (pt <= 127) {
        return PayloadType(pt);
    }
    return std::nullopt;
}


}

namespace std {
    template<>
    struct hash<freewebrtc::rtp::PayloadType> {
        size_t operator()(const freewebrtc::rtp::PayloadType& pt) const {
            return hash<uint8_t>()(pt.value());
        }
    };
}
