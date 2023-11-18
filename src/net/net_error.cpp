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
        case Error::OK:  return "success";
        case Error::INVALID_ADDRESS_SIZE: return "invalid data size to initialize address";
        case Error::IP_ADDRESS_PARSE_ERROR: return "cannot parse IP address";
        }
        return "unknown stun error";
    }
};

const std::error_category& net_error_category() noexcept {
    static const ErrorCategory cat;
    return cat;
}

}
