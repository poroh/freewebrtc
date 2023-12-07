//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Network library error codes
//

#include "net/net_error.hpp"

namespace freewebrtc::net {

class ErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "net error";
    }
    std::string message(int code) const override {
        switch ((Error)code) {
        case Error::ok:  return "success";
        case Error::invalid_address_size: return "invalid data size to initialize address";
        case Error::ip_address_parse_error: return "cannot parse IP address";
        case Error::fqdn_invalid_label_expect_letter: return "invalid FQDN label: expect letter";
        case Error::fqdn_not_fully_parsed: return "FQDN is not fully parsed";
        }
        return "unknown stun error";
    }
};

const std::error_category& net_error_category() noexcept {
    static const ErrorCategory cat;
    return cat;
}

}
