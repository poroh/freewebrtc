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
    static ReturnValue<Foundation> from_string(std::string_view);

private:
    std::string m_value;
};

}
