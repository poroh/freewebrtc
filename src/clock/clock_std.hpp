//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Clock generation from std::chrono
//

#pragma once

#include <chrono>
#include <optional>
#include <thread>

#include "clock/clock_timepoint.hpp"

namespace freewebrtc::clock {

template<typename ChronoClock>
class ChronoClockConverter {
public:
    ChronoClockConverter();

    Timepoint now();
private:
    using ChronoTimepoint = typename ChronoClock::time_point;
    using ChronoDuration  = typename ChronoClock::duration;
    ChronoTimepoint m_prev;
    ChronoDuration m_reminder{0};
    Timepoint m_now;
};

Timepoint steady_clock_now() noexcept;

//
// inlines
//
template<typename ChronoClock>
ChronoClockConverter<ChronoClock>::ChronoClockConverter()
    : m_prev(ChronoClock::now())
    , m_reminder(m_prev - ChronoTimepoint{ChronoDuration{0}})
    , m_now(Timepoint::epoch())
{
    m_now = m_now.advance_from(m_reminder);
}

template<typename ChronoClock>
Timepoint ChronoClockConverter<ChronoClock>::now() {
    auto now = ChronoClock::now();
    m_reminder += (now - m_prev);
    m_prev = now;
    m_now = m_now.advance_from(m_reminder);
    return m_now;
}

inline Timepoint steady_clock_now() noexcept {
    static thread_local ChronoClockConverter<std::chrono::steady_clock> converter;
    return converter.now();
}

}


