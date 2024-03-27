//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Foundation
//

#pragma once

#include <string>
#include "util/util_result.hpp"

namespace freewebrtc::ice::candidate {

class Foundation {
public:
    Foundation(const Foundation&) = default;
    Foundation(Foundation&&) = default;
    Foundation& operator=(const Foundation&) = default;
    Foundation& operator=(Foundation&&) = default;

    static Result<Foundation> from_string(const std::string_view&);

    const std::string& to_string() const noexcept;
    bool operator==(const Foundation&) const noexcept = default;

private:
    Foundation(const std::string_view&);
    std::string m_value;
};

//
// implementation
//
inline Foundation::Foundation(const std::string_view& v)
    : m_value(v)
{}

inline const std::string& Foundation::to_string() const noexcept {
    return m_value;
}

}
