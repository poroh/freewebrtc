//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE library error codes
//

#pragma once

#include <system_error>

namespace freewebrtc::ice::candidate {

enum class Error {
    ok = 0,
    invalid_attr_prefix,
    invalid_candidate_parts_number,
    candidate_type_absent,
    unknown_candidate_type,
    invalid_foundation_length,
    invalid_foundation_char,
    invalid_component_id_length,
    invalid_component_id_char,
    invalid_component_id_value,
    unknown_transport_type,
    invalid_priority_length,
    invalid_priority_value,
    invalid_type_preference_value,
    invalid_component_preference_value,
    invalid_local_preference_value,
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
