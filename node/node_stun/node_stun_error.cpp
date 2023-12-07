//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Node STUN error code
//

#include "node/node_stun/node_stun_error.hpp"

namespace freewebrtc::node_stun {

namespace {

const ErrorCategory& node_stun_category() {
    static const ErrorCategory cat;
    return cat;
}

}

const char *ErrorCategory::name() const noexcept {
    return "node stun";
}

std::string ErrorCategory::message(int code) const {
    switch ((Error)code) {
    case Error::ok:            return "Success";
    case Error::unknown_stun_method: return "Unknown stun method";
    case Error::unknown_stun_class: return "Unknown stun class";
    case Error::invalid_ip_address:  return "Invalid IP address";
    case Error::invalid_port_number: return "Invalid port number";
    }
    return "Unknown node stun code: " + std::to_string(code);
}

std::error_code make_error_code(Error code) {
    return std::error_code((int)code, node_stun_category());
}

}
