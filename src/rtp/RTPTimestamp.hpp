//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Timestamp
//

#pragma once

#include <cstdint>
#include <variant>

#include "rtp/RTPClock.hpp"

namespace freewebrtc::rtp {

class Timestamp {
public:
    static Timestamp from_uint32(uint32_t, RTPClock clock);

private:
    using AudioUWB = std::chrono::time_point<AudioUWBClock>;
    using AudioWB = std::chrono::time_point<AudioWBClock>;
    using AudioNB = std::chrono::time_point<AudioNBClock>;
    using Video = std::chrono::time_point<VideoClock>;

    using Value = std::variant<AudioUWB, AudioWB, AudioNB, Video>;
    Value m_value;
};

}
