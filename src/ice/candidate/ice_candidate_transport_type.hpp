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
    TransportType(const TransportType&) = default;
    TransportType(TransportType&&) = default;
    TransportType& operator=(const TransportType&) = default;
    TransportType& operator=(TransportType&&) = default;

    enum Value {
        UDP,
    };
    static TransportType udp() noexcept;

    bool operator==(const TransportType&) const noexcept = default;

    static ReturnValue<TransportType> from_string(const std::string_view&) noexcept;
    const std::string& to_string() const;

private:
    explicit TransportType(Value);
    Value m_value;
};

//
// implementation
//
inline TransportType::TransportType(Value v)
    : m_value(v)
{}

inline TransportType TransportType::udp() noexcept {
    return TransportType(UDP);
}

}
