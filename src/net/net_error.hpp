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
    OK = 0,
    INVALID_ADDRESS_SIZE,
    IP_ADDRESS_PARSE_ERROR,
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
