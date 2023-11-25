//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Retransmit timeout (RTO) calculation for STUN UDP Client
//
// Uses RFC6298 algorithm for each pair of IP addresses (source / target)
// See also:
// 1. Karn P. and Partridge C. "Improving Round-Trip Time Estimates in Reliable Transport Protocols"
//    (http://ccr.sigcomm.org/archive/1995/jan95/ccr-9501-partridge87.pdf)
// 2. Jacobson V. and Karels M. "Congestion Avoidance and Control"
//    (https://ee.lbl.gov/papers/congavoid.pdf)
//

#pragma once

#include <queue>
#include <unordered_map>

#include "clock/clock_timepoint.hpp"
#include "net/net_path.hpp"
#include "net/net_path_hash.hpp"
#include "stun/stun_client_udp_settings.hpp"

namespace freewebrtc::stun::details {

class ClientUDPRtoCalculator {
public:
    using Duration = clock::NativeDuration;
    using Timepoint = clock::Timepoint;
    using Settings = client_udp::Settings::RtoCalculatorSettings;
    explicit ClientUDPRtoCalculator(const Settings&);

    // Get initial RTO for the network path.
    Duration rto(const net::Path&) const;
    // Update RTT (received without retransmits)
    void new_rtt(Timepoint now, const net::Path&, Duration rtt);
    // Update RTO to backoff timeout if sent with retransmits.
    void backoff(Timepoint now, const net::Path&, Duration backoff);

private:
    struct Data {
        Timepoint last_update;
        struct SmoothVals {
            Duration srtt;
            Duration rttvar;
        };
        std::optional<SmoothVals> smooth;
        std::optional<Duration> backoff;
    };
    using TimelineItem = std::pair<Timepoint, net::Path>;
    struct TimelineGreater {
        bool operator()(const TimelineItem&, const TimelineItem&) const noexcept;
    };
    using ByPath = std::unordered_map<net::Path, Data, net::PathHash>;

    const Settings m_settings;
    ByPath   m_by_path;
};

}


