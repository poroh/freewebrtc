//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE library error codes
//

#pragma once

#include <system_error>

namespace freewebrtc::ice::pair {

enum class Error {
    ok = 0,
    ice_pair_state_unexpected_event,
};

std::error_code make_error_code(Error) noexcept;

const std::error_category& ice_error_category() noexcept;

//
// inline
//
inline std::error_code make_error_code(Error ec) noexcept {
    return std::error_code((int)ec, ice_error_category());
}

}
