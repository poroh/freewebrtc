//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Priority
//

#include "abnf/abnf.hpp"
#include "ice/candidate/ice_candidate_priority.hpp"
#include "ice/candidate/ice_candidate_error.hpp"

namespace freewebrtc::ice::candidate {

Result<Priority> Priority::from_uint32(uint32_t v) noexcept {
    if (v == 0) {
        return make_error_code(Error::invalid_priority_value);
    }
    return Priority(v);
}

Result<Priority> Priority::from_string(const std::string_view& v) noexcept {
    // priority = 1*10DIGIT
    if (v.empty() || v.size() > 10) {
        return make_error_code(Error::invalid_priority_length);
    }
    uint64_t result = 0;
    for (auto c: v) {
        if (!abnf::is_DIGIT(c)) {
            return make_error_code(Error::invalid_priority_value);
        }
        result = 10 * result + c - '0';
    }
    // is a positive integer between 1 and (2**31 - 1) inclusive. The
    // procedures for computing a candidate's priority are described
    // in Section 5.1.2 of [RFC8445].
    if (result == 0 || result > ((1ul << 32) - 1)) {
        return make_error_code(Error::invalid_priority_value);
    }
    return Priority(result);
}

}
