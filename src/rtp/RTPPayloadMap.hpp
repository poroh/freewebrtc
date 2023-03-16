//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Map
//

#pragma once

#include "rtp/RTPClock.hpp"
#include <unordered_map>

namespace freewebrtc::rtp {

struct PayloadMapItem {
    RTPClock clock;
};

// This is equivalent of series of a=rtpmap: attributes in SDP
class PayloadMap {
public:
    std::optional<RTPClock> rtp_clock_rate(PayloadType) const noexcept;
private:
    std::unordered_map<PayloadType, PayloadMapItem> m_items;
};

}
