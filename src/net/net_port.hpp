
//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Network port (TCP / UDP / SCTP / ...)
//

#pragma once

#include <cstdint>

namespace freewebrtc::net {

class Port {
public:
    explicit Port(uint16_t);
private:
    uint16_t m_value;
};

};
