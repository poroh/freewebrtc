//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Retransmit timeout (RTO) calculation for STUN UDP Client

#include <cmath>
#include "util/util_fmap.hpp"
#include "stun/details/stun_client_udp_rto.hpp"


namespace freewebrtc::stun::details {

using Duration = ClientUDPRtoCalculator::Duration;

ClientUDPRtoCalculator::ClientUDPRtoCalculator(const Settings& settings)
    : m_settings(settings)
{}

Duration ClientUDPRtoCalculator::rto(const net::Path& path) const {
    auto it = m_by_path.find(path);
    if (it == m_by_path.end()) {
        return m_settings.initial_rto;
    }
    const auto& data = it->second;
    static constexpr unsigned K = 4;
    // Karn's algorithm:
    // Use previous backoff for further RTO until
    // we get reliable RTT value.
    return data.backoff
        .value_or(
            util::fmap(data.smooth, [](auto&& s) { return s.srtt + K * s.rttvar; })
                .value_or(m_settings.initial_rto));
}

void ClientUDPRtoCalculator::new_rtt(Timepoint now, const net::Path& path, Duration rtt) {
    auto it = m_by_path.find(path);
    if (it == m_by_path.end()) {
        auto [it2, _] = m_by_path.emplace(path, Data{now});
        it = it2;
    }
    auto& data = it->second;
    data.last_update = now;
    // Clear backoff value for further requests
    data.backoff.reset();
    // Update SRTT
    if (!it->second.smooth.has_value()) {
        // SRTT <- R
        // RTTVAR <- R/2
        it->second.smooth.emplace(Data::SmoothVals{rtt, rtt / 2});
        return;
    }
    auto& smooth = data.smooth.value();
    auto& rttvar = smooth.rttvar;
    auto& srtt = smooth.srtt;

    // RFC6298:
    // The above SHOULD be computed using alpha=1/8 and beta=1/4 (as
    // suggested in [JK88]).
    const constexpr std::ratio<1, 8> alpha;
    const constexpr std::ratio<1, 4> beta;
    auto srtt_delta = srtt - rtt;
    if (srtt_delta.count() < 0) {
        srtt_delta = -srtt_delta;
    }
    // RFC6298:
    // RTTVAR <- (1 - beta) * RTTVAR + beta * |SRTT - R'|
    rttvar = ((beta.den - beta.num) * rttvar + beta.num * srtt_delta) / beta.den;
    // SRTT <- (1 - alpha) * SRTT + alpha * R'
    srtt = ((alpha.den - alpha.num) * srtt + alpha.num * rtt) / alpha.den;
}

void ClientUDPRtoCalculator::backoff(Timepoint now, const net::Path& path, Duration backoff) {
    auto it = m_by_path.find(path);
    if (it == m_by_path.end()) {
        m_by_path.emplace(path, Data{now, std::nullopt, backoff});
        return;
    }
    auto& data = it->second;
    data.last_update = now;
    data.backoff = backoff;
}

}



