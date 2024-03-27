//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Component Identifier
//

#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "util/util_result.hpp"

namespace freewebrtc::ice::candidate {

class ComponentId {
public:
    ComponentId(const ComponentId&) = default;
    ComponentId(ComponentId&&) = default;
    ComponentId& operator=(const ComponentId&) = default;
    ComponentId& operator=(ComponentId&&) = default;

    static Result<ComponentId> from_unsigned(unsigned) noexcept;
    static Result<ComponentId> from_string(const std::string_view&) noexcept;

    bool operator==(const ComponentId&) const noexcept = default;
    unsigned value() const noexcept;

private:
    explicit ComponentId(unsigned);
    unsigned m_value;
};

using MaybeComponentId = std::optional<ComponentId>;

//
// implementation
//
inline ComponentId::ComponentId(unsigned v)
    : m_value(v)
{}

inline unsigned ComponentId::value() const noexcept {
    return m_value;
}

}
