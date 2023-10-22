//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// NAPI status to std::error_code
//

#include "napi_error.hpp"

namespace freewebrtc::napi {

namespace {

const NapiErrorCategory& napi_category() {
    static const NapiErrorCategory cat;
    return cat;
}

const NapiWrapperErrorCategory& napi_wrapper_category() {
    static const NapiWrapperErrorCategory cat;
    return cat;
}

}

const char *NapiErrorCategory::name() const noexcept {
    return "NodeNAPI";
}

std::string NapiErrorCategory::message(int code) const {
    switch (code) {
    case napi_ok:                  return "Success";
    case napi_invalid_arg:         return "Invalid pointer passed as argument";
    case napi_object_expected:     return "An object was expected";
    case napi_string_expected:     return "A string was expected";
    case napi_name_expected:       return "A string or symbol was expected";
    case napi_function_expected:   return "A function was expected";
    case napi_number_expected:     return "A number was expected";
    case napi_boolean_expected:    return "A boolean was expected";
    case napi_array_expected:      return "An array was expected";
    case napi_generic_failure:     return "Unknown failure";
    case napi_pending_exception:   return "An exception is pending";
    case napi_cancelled:           return "The async work item was cancelled";
    case napi_escape_called_twice: return "napi_escape_handle already called on scope";
    }
    return "Unknown napi status: " + std::to_string(code);
}

std::error_code make_error_code(napi_status code) {
    return std::error_code(code, napi_category());
}

const char *NapiWrapperErrorCategory::name() const noexcept {
    return "NAPIWrapper";
}

std::string NapiWrapperErrorCategory::message(int code) const {
    switch ((WrapperError)code) {
    case WrapperError::OK:            return "Success";
    case WrapperError::INVALID_TYPE:  return "Invalid type";
    case WrapperError::UNKNOWN_STUN_METHOD: return "Unknown stun method";
    }
    return "Unknown napi wrapper code: " + std::to_string(code);
}

std::error_code make_error_code(WrapperError code) {
    return std::error_code((int)code, napi_wrapper_category());
}

}
