//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Network port (TCP / UDP / SCTP / ...)
//

#include "abnf/abnf.hpp"
#include "net/net_port.hpp"
#include "net/net_error.hpp"

namespace freewebrtc::net {

ReturnValue<Port> Port::from_string(const std::string_view& v) noexcept {
    // RFC4566:
    // port = 1*DIGIT
    if (v.empty() || v.size() > 5) { // 16 bit 65536
        return make_error_code(Error::invalid_port_value);
    }
    uint32_t result = 0;
    for (auto c: v) {
        if (!abnf::is_DIGIT(c)) {
            return make_error_code(Error::invalid_port_value);
        }
        result = 10 * result + c - '0';
    }
    if (result > 0xFFFF) {
        return make_error_code(Error::invalid_port_value);
    }
    return Port(result);
}

}
