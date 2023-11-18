//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Time point that is used everywhere inside the library
// Rationality about introduction of own time_point
// is that not everybody want to use std::chrono::stready_clock
// or any other specific clock. Also std::chrono::stready_clock::time_point
// is integer-based and overflow is UB.
//
// So freewebrtc introduce its own timepoint. More rigit than chrono one.
// Intenal frequency of this clock is 10^6.
//

#pragma once

#include <cstdint>
#include <chrono>
#include <type_traits>

namespace freewebrtc::clock {

class Timepoint {
    using Base = uint64_t;
    using SignedBase = std::make_signed<Base>::type;
    using Ratio = std::micro;
    using UnderlyingDuration = std::chrono::duration<Base, Ratio>;
public:
    using Duration = std::chrono::duration<SignedBase, Ratio>;
    Timepoint(const Timepoint&) = default;
    Timepoint(Timepoint&&) = default;
    Timepoint& operator=(const Timepoint&) = default;
    Timepoint& operator=(Timepoint&&) = default;

    // Create epoch time
    static Timepoint epoch() noexcept;

    // Function return timepoint that shifted the new time
    // and subtracts from duration parameter. Reminder will
    // remain in parmeter
    template<typename ChronoDuration>
    Timepoint advance_from(ChronoDuration&) const noexcept;

    // Advance by timempoints without lost of precision
    // (microseconds, milliseconds, seconds, ...)
    template<typename ChronoDuration>
    Timepoint advance(ChronoDuration) const noexcept;

    // Two timepoints can be subtracted from each other.
    // Result is duration.
    Duration operator-(const Timepoint&) const noexcept;

    // Timeline order
    bool is_after(const Timepoint&) const noexcept;
    bool is_before(const Timepoint&) const noexcept;
    bool operator==(const Timepoint&) const noexcept = default;

private:
    explicit Timepoint(const UnderlyingDuration&);
    UnderlyingDuration m_value;
};

using NativeDuration = Timepoint::Duration;

//
// inlines
//
inline Timepoint::Timepoint(const UnderlyingDuration& v)
    : m_value(v)
{}

inline Timepoint Timepoint::epoch() noexcept {
    return Timepoint{UnderlyingDuration(0)};
}

template<typename ChronoDuration>
inline Timepoint Timepoint::advance_from(ChronoDuration& duration) const noexcept {
    auto advance = std::chrono::duration_cast<UnderlyingDuration>(duration);
    duration -= std::chrono::duration_cast<ChronoDuration>(advance);
    return Timepoint{m_value + advance};
}

template<typename ChronoDuration>
Timepoint Timepoint::advance(ChronoDuration duration) const noexcept {
    using InPeriod = typename ChronoDuration::period;
    using TpPeriod = Duration::period;
    // guarnatee that duration_cast will not implicitly lost data on devision
    static_assert((TpPeriod::den % InPeriod::den) == 0);
    auto advance = std::chrono::duration_cast<UnderlyingDuration>(duration);
    return Timepoint{m_value + advance};
}


inline Timepoint::Duration Timepoint::operator-(const Timepoint& other) const noexcept {
    if (this->is_after(other)) {
        return UnderlyingDuration{(m_value - other.m_value).count()};
    } else {
        return -UnderlyingDuration{(other.m_value - m_value).count()};
    }
}

inline bool Timepoint::is_after(const Timepoint& other) const noexcept {
    return other.m_value.count() - m_value.count() > std::numeric_limits<Base>::max() / 2;
}

inline bool Timepoint::is_before(const Timepoint& other) const noexcept {
    return m_value.count() - other.m_value.count() > std::numeric_limits<Base>::max() / 2;
}

}
