//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Data structures
//

#pragma once

#include <vector>

#include "rtp/RTPMarker.hpp"
#include "rtp/RTPPayloadType.hpp"
#include "rtp/RTPTimestamp.hpp"
#include "rtp/RTPSequence.hpp"
#include "rtp/RTPSSRC.hpp"
#include "util/UtilBinaryView.hpp"

namespace freewebrtc::rtp {

struct Header {
    MarkerBit marker;
    PayloadType payload_type;
    SequenceNumber sequence;
    SSRC ssrc;
    Timestamp timestamp;
    std::vector<SSRC> cssrc;
    struct Extension {
        uint16_t profile_defined;
        util::ConstBinaryView::Interval data;
    };
    std::optional<Extension> extension;
};

}
