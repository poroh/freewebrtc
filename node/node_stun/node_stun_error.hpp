//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Node STUN error code
//

#pragma once

#include <system_error>

namespace freewebrtc::node_stun {

enum class Error {
    ok = 0,
    unknown_stun_method,
    unknown_stun_class,
    invalid_ip_address,
    invalid_port_number
};

class ErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override;
    std::string message(int code) const override;
};

std::error_code make_error_code(Error error);

}
