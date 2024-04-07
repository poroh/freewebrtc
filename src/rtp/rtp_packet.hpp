//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Packet-related functions
//

#pragma once

#include "util/util_typed_bool.hpp"
#include "util/util_binary_view.hpp"
#include "util/util_result.hpp"
#include "stat/stat_counter.hpp"
#include "rtp/rtp_header.hpp"

namespace freewebrtc::rtp {

class PayloadMap;

struct ParseStat {
    stat::Counter success;
    stat::Counter error;
    stat::Counter invalid_size;
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
    static Result<Packet> parse(const util::ConstBinaryView&, const PayloadMap&, ParseStat&) noexcept;
};


}
