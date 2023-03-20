
//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Attribute container
//

#pragma once

#include <unordered_map>

#include "stun/stun_attribute.hpp"

namespace freewebrtc::stun {

class AttributeSet {
public:
    void emplace(Attribute&&);
private:
    std::unordered_map<AttributeType, Attribute> m_map;
};


//
// inlines
//
inline void AttributeSet::emplace(Attribute&& attr)  {
    if (m_map.find(attr.type()) != m_map.end()) {
        return;
    }
    m_map.emplace(attr.type(), std::move(attr));
}


}
