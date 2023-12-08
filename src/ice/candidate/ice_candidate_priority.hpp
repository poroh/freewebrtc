//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Priority
//

#pragma once

#include <cstdint>
#include <string_view>
#include "util/util_return_value.hpp"

namespace freewebrtc::ice::candidate {

class Priority {
public:
    static Priority from_uint32(uint32_t);
    static ReturnValue<Priority> from_string(const std::string_view&) noexcept;

private:
    uint32_t m_value;
};


}
