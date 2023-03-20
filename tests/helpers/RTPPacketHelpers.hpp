//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Packet Helpers for unit tests
//

#pragma once

#include <cstdint>
#include <vector>

#include "rtp/details/RTPHeaderDetails.hpp"
#include "util/UtilFlat.hpp"
#include "EndianHelpers.hpp"

namespace freewebrtc::test::rtp_helpers {

std::vector<uint8_t> first_word(unsigned pt, unsigned seqnum, bool marker = false, bool padding = false, bool extension = false, uint8_t num_cssrc = 0);
std::vector<uint8_t> extension_header(uint16_t profile_specific, size_t len);

//
// inlines
//
inline std::vector<uint8_t> first_word(unsigned pt, unsigned seqnum, bool marker, bool padding, bool extension, uint8_t num_cssrc) {
    using namespace freewebrtc::rtp::details;
    const uint8_t first_byte
        = (RTP_VERSION << RTP_VERSION_SHIFT)
        | (padding ? RTP_PADDING_MASK : 0)
        | (extension ? RTP_EXTENSION_MASK : 0)
        | num_cssrc;
    const uint8_t second_byte
        = (pt & RTP_PAYLOAD_TYPE_MASK)
        | (marker ? RTP_MARKER_MASK : 0);
    return {
        first_byte, second_byte,
        (uint8_t)((seqnum >> 8) & 0xff),
        (uint8_t)seqnum
    };
}

inline std::vector<uint8_t> extension_header(uint16_t profile_specific, size_t len) {
    return util::flat_vec<uint8_t>({
            helpers::uint16be(profile_specific),
            helpers::uint16be((uint16_t)len)
        });
}



}

