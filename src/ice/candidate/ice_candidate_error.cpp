//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE candidate library error codes
//

#include "ice/candidate/ice_candidate_error.hpp"

namespace freewebrtc::ice::candidate {

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
    return "ice candidate error";
}

std::string IceCandidateError::message(int code) const {
    switch ((Error)code) {
    case Error::ok: return "Success";
    case Error::invalid_attr_prefix: return "Invalid attribute prefix";
    case Error::invalid_candidate_parts_number: return "Invalid candidate string number of tokens";
    case Error::candidate_type_absent:  return "Candidate type is absent in candidate string";
    case Error::unknown_candidate_type:  return "Candidate type is unknown";
    case Error::invalid_foundation_length: return "Invalid length of foundation string";
    case Error::invalid_foundation_char: return "Invalid char in foundation string";
    case Error::invalid_component_id_length: return "Invalid length of component id";
    case Error::invalid_component_id_char: return "Invalid char in component id";
    case Error::invalid_component_id_value: return "Invalid value of component id";
    case Error::unknown_transport_type: return "Transport type is unknown";
    case Error::invalid_priority_length: return "Invalid candidate priority length";
    case Error::invalid_priority_value: return "Invalid candidate priority value";
    case Error::invalid_type_preference_value: return "Invalid candidate type preference value";
    }
    return "Unknown ice candidate error: " + std::to_string(code);
}

}
