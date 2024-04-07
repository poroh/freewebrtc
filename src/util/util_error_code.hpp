//
// Copyright (c) 2024 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Util errors
//

#include <system_error>

namespace freewebrtc::util {

enum class ErrorCode {
    ok = 0,
    value_required_in_maybe,
};

std::error_code make_error_code(ErrorCode) noexcept;

const std::error_category& util_error_category() noexcept;

//
// inlines
//
inline std::error_code make_error_code(ErrorCode ec) noexcept {
    return std::error_code((int)ec, util_error_category());
}

}
