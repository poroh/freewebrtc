//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Packet Parser
//

#pragma once

#include "util/UtilTypedBool.hpp"
#include "util/UtilBinaryView.hpp"
#include "stat/StatCounter.hpp"
#include "rtp/RTPHeader.hpp"

namespace freewebrtc::rtp {

class PayloadMap;

struct ParseStat {
    stat::Counter success;
    stat::Counter error;
    stat::Counter truncated;
    stat::Counter invalid_version;
    stat::Counter invalid_csrc;
    stat::Counter invalid_extension;
    stat::Counter invalid_payload_type;
    stat::Counter unknown_rtp_clock;
    stat::Counter invalid_padding;
};

struct Packet {
    Header header;
    util::ConstBinaryView::Interval payload;
    static std::optional<Packet> parse(const util::ConstBinaryView&, const PayloadMap&, ParseStat&) noexcept;
};


}
