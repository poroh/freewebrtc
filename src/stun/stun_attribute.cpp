
//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Attribute
//

#include "stun/stun_attribute.hpp"
#include "stun/details/stun_attr_registry.hpp"

namespace freewebrtc::stun {

UnknownAttribute::UnknownAttribute(const util::ConstBinaryView& vv, uint16_t t)
    : type(t)
    , data(vv.begin(), vv.end())
{}


Attribute::Attribute(Value&& v)
    : m_value(std::move(v))
{}

std::optional<Attribute> Attribute::parse(const util::ConstBinaryView& vv, uint16_t type, ParseStat& stat) {
    switch (type) {
        case attr_registry::MAPPED_ADDRESS:
            if (auto maybe_attr = MappedAddressAttribute::parse(vv, stat); maybe_attr.has_value()) {
                return Attribute(*maybe_attr);
            }
            return std::nullopt;
        default:
            return Attribute(UnknownAttribute(vv, type));
    }
}

}
