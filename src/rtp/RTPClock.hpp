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

static constexpr uint32_t AUDIO_UWB_FREQ = 48000;
static constexpr uint32_t AUDIO_WB_FREQ = 16000;
static constexpr uint32_t AUDIO_NB_FREQ = 8000;
static constexpr uint32_t VIDEO_FREQ = 90000;

using AudioUWBRatio = std::ratio<1, AUDIO_UWB_FREQ>;
using AudioWBRatio = std::ratio<1, AUDIO_WB_FREQ>;
using AudioNBRatio = std::ratio<1, AUDIO_NB_FREQ>;
using VideoRatio = std::ratio<1, VIDEO_FREQ>;

template<typename Ratio>
struct RTPClockTpl {
    using rep = uint32_t;
    using period = Ratio;
    using duration = std::chrono::duration<rep, period>;
    bool is_steady = false;
};

using AudioUWBClock = RTPClockTpl<AudioUWBRatio>;
using AudioWBClock = RTPClockTpl<AudioWBRatio>;
using AudioNBClock = RTPClockTpl<AudioNBRatio>;
using VideoClock = RTPClockTpl<VideoRatio>;

using RTPClock = std::variant<
    AudioUWBClock,
    AudioWBClock,
    AudioNBClock,
    VideoClock>;

}
