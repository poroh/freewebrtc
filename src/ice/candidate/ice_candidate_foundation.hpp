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
#include "util/util_return_value.hpp"

namespace freewebrtc::ice::candidate {

class Foundation {
public:
    static ReturnValue<Foundation> from_string(const std::string_view&);

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

}
