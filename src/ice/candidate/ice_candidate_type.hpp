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

class Type {
public:
    enum Value {
        HOST,
        SERVER_REFLEXIVE,
        PEER_REFLEXIVE,
        RELAYED
    };
    static Type host();
    static Type server_reflexive();
    static Type peer_reflexive();
    static Type relayed();

    static ReturnValue<Type> from_string(const std::string_view&) noexcept;

private:
    Value m_value;
};

}
