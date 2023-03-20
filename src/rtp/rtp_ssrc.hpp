//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Synchronization Source (AKA SSRC)
//

#pragma once

#include <cstdint>

namespace freewebrtc::rtp {

class SSRC {
public:
    static SSRC from_uint32(uint32_t);
    uint32_t value() const noexcept;
    bool operator==(const SSRC&) const noexcept;
private:
    explicit SSRC(uint32_t);
    uint32_t m_value;
};

//
// inlines
//
inline SSRC::SSRC(uint32_t v)
    : m_value(v)
{}

inline SSRC SSRC::from_uint32(uint32_t v) {
    return SSRC{v};
}

inline uint32_t SSRC::value() const noexcept {
    return m_value;
}

inline bool SSRC::operator==(const SSRC& other) const noexcept {
    return m_value == other.m_value;
}


}
