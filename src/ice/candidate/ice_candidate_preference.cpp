//
// Copyright (c) 2024 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE candidate prefernce values:
// https://datatracker.ietf.org/doc/html/rfc5245#section-4.1.2.1
//

#include "ice/candidate/ice_candidate_preference.hpp"
#include "ice/candidate/ice_candidate_error.hpp"

namespace freewebrtc::ice::candidate {

TypePreference::TypePreference(unsigned v) noexcept
    : m_value(v)
{}

TypePreference TypePreference::recommended_for(Type t) noexcept {
    // RFC5245:
    // The RECOMMENDED values are 126 for host candidates, 100 for
    // server reflexive candidates, 110 for peer reflexive candidates,
    // and 0 for relayed candidates.
    switch (t.value()) {
    case Type::HOST:
        return Self{126};
    case Type::SERVER_REFLEXIVE:
        return Self{100};
    case Type::PEER_REFLEXIVE:
        return Self{110};
    case Type::RELAYED:
        return Self{0};
    }
    return Self{0};
}

Result<TypePreference> TypePreference::from_unsigned(unsigned v) noexcept {
    if (v > 126) {
        return make_error_code(Error::invalid_type_preference_value);
    }
    return Self{v};
}

Result<Priority> Preference::to_priority() const noexcept {
    unsigned p = (type.value() << 24)
        | (local.value() << 8)
        | component.value();
    return Priority::from_uint32(p);
}

}
