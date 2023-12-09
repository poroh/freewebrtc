//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Foundation
//

#include "ice/candidate/ice_candidate_component_id.hpp"
#include "ice/candidate/ice_candidate_error.hpp"
#include "abnf/abnf.hpp"

namespace freewebrtc::ice::candidate {

ReturnValue<ComponentId> ComponentId::from_unsigned(unsigned v) noexcept {
    if (v >= 1000) {
        return make_error_code(Error::invalid_component_id_value);
    }
    return ComponentId(v);
}

ReturnValue<ComponentId> ComponentId::from_string(const std::string_view& v) noexcept {
    // component-id = 1*3DIGIT
    if (v.empty() || v.size() > 3) {
        return make_error_code(Error::invalid_component_id_length);
    }
    for (auto c: v) {
        if (!freewebrtc::abnf::is_DIGIT(c)) {
            return make_error_code(Error::invalid_component_id_char);
        }
    }
    std::string vstr{v};
    auto value = std::atoi(vstr.c_str());
    return ComponentId(value);
}

}

