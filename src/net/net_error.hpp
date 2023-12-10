//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Network library error codes
//

#pragma once

#include <system_error>

namespace freewebrtc::net {

enum class Error {
    ok = 0,
    invalid_address_size,
    ip_address_parse_error,
    fqdn_invalid_label_expect_letter,
    fqdn_not_fully_parsed,
    invalid_port_value
};

std::error_code make_error_code(Error) noexcept;

const std::error_category& net_error_category() noexcept;

//
// inline
//
inline std::error_code make_error_code(Error ec) noexcept {
    return std::error_code((int)ec, net_error_category());
}


}
