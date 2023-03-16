//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Payload Type
//

#pragma once

#include <cstdint>
#include <optional>

namespace freewebrtc::rtp {

class PayloadType {
public:
    static std::optional<PayloadType> from_uint8(uint8_t pt);

private:
    uint8_t m_value;
};

}
