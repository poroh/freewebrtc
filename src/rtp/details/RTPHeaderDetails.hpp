//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Header-related constants
//

#pragma once

#include <cstdint>
#include <cstddef>

namespace freewebrtc::rtp::details {

//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |V=2|P|X|  CC   |M|     PT      |       sequence number         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           timestamp                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           synchronization source (SSRC) identifier            |
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |            contributing source (CSRC) identifiers             |
// |                             ....                              |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
static constexpr size_t  RTP_FIXED_HEADER_LEN = 12;
static constexpr uint8_t RTP_VERSION = 2;
static constexpr uint8_t RTP_VERSION_MASK = 0xC0;
static constexpr int     RTP_VERSION_SHIFT = 6;
static constexpr uint8_t RTP_PADDING_MASK = 0x20;
static constexpr uint8_t RTP_EXTENSION_MASK = 0x10;
static constexpr uint8_t RTP_CC_MASK = 0x0F;
static constexpr uint8_t RTP_MARKER_MASK = 0x80;
static constexpr uint8_t RTP_PAYLOAD_TYPE_MASK = 0x7F;
static constexpr size_t  RTP_SEQUENCE_NUMBER_OFFSET = 2;
static constexpr size_t  RTP_TIMESTAMP_OFFSET = 4;
static constexpr size_t  RTP_SSRC_OFFSET = 8;

}


