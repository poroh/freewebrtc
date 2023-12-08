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
#include <optional>
#include <string>

#include "util/util_return_value.hpp"

namespace freewebrtc::ice::candidate {

class ComponentId {
public:
    static ComponentId from_unsigned(unsigned);
    static ReturnValue<ComponentId> from_string(const std::string_view&) noexcept;

private:
    unsigned m_value;
};

using MaybeComponentId = std::optional<ComponentId>;

}
