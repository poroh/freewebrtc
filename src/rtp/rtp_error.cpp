//
// Copyright (c) 2024 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Errors
//

#include "rtp/rtp_error.hpp"

namespace freewebrtc::rtp {

class ErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "net error";
    }
    std::string message(int code) const override {
        switch ((Error)code) {
        case Error::ok:  return "success";
        case Error::packet_is_too_short: return "rtp packet is too short";
        case Error::unknown_packet_version: return "rtp packet version is unknown";
        case Error::invalid_payload_type: return "rtp packet with invalid payload type";
        case Error::unknown_rtp_clock: return "unknown rtp clock rate";
        case Error::invalid_extension_length: return "invalid extension length";
        case Error::invalid_packet_padding: return "invalid packet padding";
        }
        return "unknown rtp error";
    }
};

const std::error_category& rtp_error_category() noexcept {
    static const ErrorCategory cat;
    return cat;
}

}

