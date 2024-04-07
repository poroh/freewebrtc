//
// Copyright (c) 2024 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Errors
//

#pragma once

#include <system_error>

namespace freewebrtc::rtp {

enum class Error {
    ok = 0,
    packet_is_too_short,
    unknown_packet_version,
    invalid_payload_type,
    unknown_rtp_clock,
    invalid_extension_length,
    invalid_packet_padding,
};

std::error_code make_error_code(Error) noexcept;

const std::error_category& rtp_error_category() noexcept;

//
// inline
//
inline std::error_code make_error_code(Error ec) noexcept {
    return std::error_code((int)ec, rtp_error_category());
}

}

