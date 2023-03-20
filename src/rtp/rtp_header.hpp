//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Packet Header
//

#pragma once

#include <vector>

#include "rtp/rtp_marker.hpp"
#include "rtp/rtp_payload_type.hpp"
#include "rtp/rtp_timestamp.hpp"
#include "rtp/rtp_sequence.hpp"
#include "rtp/rtp_ssrc.hpp"
#include "util/util_binary_view.hpp"

namespace freewebrtc::rtp {

struct Header {
    MarkerBit marker;
    PayloadType payload_type;
    SequenceNumber sequence;
    SSRC ssrc;
    Timestamp timestamp;
    std::vector<SSRC> csrcs;
    struct Extension {
        uint16_t profile_defined;
        util::ConstBinaryView::Interval data;
    };
    std::optional<Extension> extension;
};

}
