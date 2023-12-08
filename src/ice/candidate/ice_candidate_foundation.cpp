//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Foundation
//

#include "ice/candidate/ice_candidate_foundation.hpp"
#include "ice/candidate/ice_candidate_error.hpp"
#include "ice/ice_abnf.hpp"

namespace freewebrtc::ice::candidate {

ReturnValue<Foundation> Foundation::from_string(const std::string_view& v) {
    if (v.empty() || v.size() > 32) {
        return make_error_code(Error::invalid_foundation_length);
    }
    for (auto c: v) {
        if (!ice::abnf::is_ice_char(c)) {
            return make_error_code(Error::invalid_foundation_char);
        }
    }
    return Foundation(v);
}


}

