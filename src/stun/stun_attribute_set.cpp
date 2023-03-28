//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Attribute container
//

#include "stun/stun_attribute_set.hpp"

namespace freewebrtc::stun {

const MessageIntegityAttribute* AttributeSet::integrity() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::MESSAGE_INTEGRITY)); it != m_map.end()) {
        return it->second.as<MessageIntegityAttribute>();
    }
    return nullptr;
}

const UsernameAttribute* AttributeSet::username() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::USERNAME)); it != m_map.end()) {
        return it->second.as<UsernameAttribute>();
    }
    return nullptr;
}

}
