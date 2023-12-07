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

namespace freewebrtc::ice::candidate {

class Priority {
public:
    static Priority from_uint32(uint32_t);
private:
    uint32_t m_value;
};


}
