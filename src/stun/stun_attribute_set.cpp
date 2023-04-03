//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Attribute container
//

#include "stun/stun_attribute_set.hpp"

namespace freewebrtc::stun {

AttributeSet::MaybeAttr<MessageIntegityAttribute::Digest> AttributeSet::integrity() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::MESSAGE_INTEGRITY)); it != m_map.end()) {
        return it->second.as<MessageIntegityAttribute>()->digest;
    }
    return std::nullopt;
}

AttributeSet::MaybeAttr<precis::OpaqueString> AttributeSet::username() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::USERNAME)); it != m_map.end()) {
        return it->second.as<UsernameAttribute>()->name;
    }
    return std::nullopt;
}

AttributeSet::MaybeAttr<std::string> AttributeSet::software() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::SOFTWARE)); it != m_map.end()) {
        return it->second.as<SoftwareAttribute>()->name;
    }
    return std::nullopt;
}

AttributeSet::MaybeAttr<XorMappedAddressAttribute> AttributeSet::xor_mapped() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::XOR_MAPPED_ADDRESS)); it != m_map.end()) {
        return *it->second.as<XorMappedAddressAttribute>();
    }
    return std::nullopt;
}


}
