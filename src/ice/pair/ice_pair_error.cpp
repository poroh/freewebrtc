//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE candidate library error codes
//

#include "ice/pair/ice_pair_error.hpp"

namespace freewebrtc::ice::pair {

namespace {

class IceCandidateError : public std::error_category {
public:
    const char *name() const noexcept override;
    std::string message(int code) const override;
};

}

const std::error_category& ice_error_category() noexcept {
    static const IceCandidateError cat;
    return cat;
}

const char *IceCandidateError::name() const noexcept {
    return "ice pair error";
}

std::string IceCandidateError::message(int code) const {
    switch ((Error)code) {
    case Error::ok: return "Success";
    case Error::ice_pair_state_unexpected_event: return "Unexpected event for pair FSM";
    }
    return "Unknown ice pair error: " + std::to_string(code);
}

}
