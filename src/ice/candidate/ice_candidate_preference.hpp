//
// Copyright (c) 2024 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE candidate prefernce values:
// https://datatracker.ietf.org/doc/html/rfc5245#section-4.1.2.1
//

#include "ice/candidate/ice_candidate_type.hpp"
#include "ice/candidate/ice_candidate_component_id.hpp"
#include "ice/candidate/ice_candidate_priority.hpp"
#include "util/util_result.hpp"

namespace freewebrtc::ice::candidate {

class TypePreference {
public:
    using Self = TypePreference;
    static Self recommended_for(Type) noexcept;
    static Result<Self> from_unsigned(unsigned v) noexcept;

    unsigned value() const noexcept;

private:
    explicit TypePreference(unsigned) noexcept;
    unsigned m_value;
};

class LocalPreference {
public:
    using Self = LocalPreference;
    static Result<Self> from_unsigned(unsigned v) noexcept;

    unsigned value() const noexcept;

private:
    explicit LocalPreference(unsigned) noexcept;
    unsigned m_value;
};

class ComponentPreference {
public:
    using Self = ComponentPreference;

    static Self recommended_for(ComponentId) noexcept;
    static Result<Self> from_unsigned(unsigned v) noexcept;
    unsigned value() const noexcept;

private:
    explicit ComponentPreference(unsigned) noexcept;
    unsigned m_value;
};

struct Preference {
    Result<Priority> to_priority() const noexcept;

    TypePreference type;
    LocalPreference local;
    ComponentPreference component;
};

//
// Implementation
//
inline unsigned TypePreference::value() const noexcept {
    return m_value;
}

inline unsigned LocalPreference::value() const noexcept {
    return m_value;
}

inline unsigned ComponentPreference::value() const noexcept {
    return m_value;
}


}
