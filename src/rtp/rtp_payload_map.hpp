//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Map
//

#pragma once

#include <unordered_map>

#include "util/util_maybe.hpp"
#include "rtp/rtp_clock_rate.hpp"
#include "rtp/rtp_payload_type.hpp"


namespace freewebrtc::rtp {

struct PayloadMapItem {
    ClockRate clock_rate;
};

// This is equivalent of series of a=rtpmap: attributes in SDP
class PayloadMap {
public:
    using InitPair = std::pair<PayloadType, PayloadMapItem>;
    using PairInitializer = std::initializer_list<InitPair>;
    explicit PayloadMap(PairInitializer);
    Maybe<ClockRate> rtp_clock_rate(PayloadType) const noexcept;
private:
    std::unordered_map<PayloadType, PayloadMapItem> m_items;
};

//
// inlines
//
inline Maybe<ClockRate> PayloadMap::rtp_clock_rate(PayloadType pt) const noexcept {
    if (auto it = m_items.find(pt); it != m_items.end()) {
        return it->second.clock_rate;
    }
    return none();
}

}
