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
#include "util/util_result.hpp"

namespace freewebrtc::ice::candidate {

class Type {
public:
    Type(const Type&) = default;
    Type(Type&&) = default;
    Type& operator=(const Type&) = default;
    Type& operator=(Type&&) = default;

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

    // From SDP string (RFC8839).
    static Result<Type> from_string(const std::string_view&) noexcept;

    Value value() const noexcept;
    const std::string& to_string() const noexcept;

    bool operator==(const Type&) const noexcept = default;

private:
    Type(Value);
    Value m_value;
};

//
// implementation
//
inline Type::Type(Value v)
    : m_value(v)
{}

inline Type Type::host() {
    return Type{HOST};
}

inline Type Type::server_reflexive() {
    return Type{SERVER_REFLEXIVE};
}

inline Type Type::peer_reflexive() {
    return Type{PEER_REFLEXIVE};
}

inline Type Type::relayed() {
    return Type{RELAYED};
}

inline Type::Value Type::value() const noexcept {
    return m_value;
}

}
