//
// Copyright (c) 2024 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Util errors
//

#include "util/util_error_code.hpp"

namespace freewebrtc::util {

class ErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "net error";
    }
    std::string message(int code) const override {
        switch ((ErrorCode)code) {
        case ErrorCode::ok:  return "success";
        case ErrorCode::value_required_in_maybe: return "value required";
        }
        return "unknown util error";
    }
};

const std::error_category& util_error_category() noexcept {
    static const ErrorCategory cat;
    return cat;
}

}
