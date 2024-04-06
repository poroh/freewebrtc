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
#include "util/util_result.hpp"

namespace freewebrtc::ice::candidate {

class Priority {
public:
    Priority(const Priority&) = default;
    Priority(Priority&&) = default;
    Priority& operator=(const Priority&) = default;
    Priority& operator=(Priority&&) = default;

    static Result<Priority> from_uint32(uint32_t) noexcept;
    static Result<Priority> from_string(const std::string_view&) noexcept;

    bool operator==(const Priority&) const noexcept = default;

    uint32_t value() const noexcept;

private:
    explicit Priority(uint32_t);
    uint32_t m_value;
};

//
// inlines
//
inline Priority::Priority(uint32_t v)
    : m_value(v)
{}

inline uint32_t Priority::value() const noexcept {
    return m_value;
}

}
