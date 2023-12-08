//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Type
// RFC8845
//

#pragma once

#include <string_view>
#include "util/util_return_value.hpp"

namespace freewebrtc::ice::candidate {

class TransportType {
public:
    enum Value {
        UDP,
    };
    static TransportType udp();

    static ReturnValue<TransportType> from_string(const std::string_view&) noexcept;

private:
    Value m_value;
};

}
