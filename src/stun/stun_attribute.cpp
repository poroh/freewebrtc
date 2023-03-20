
//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Attribute
//

#include "stun/stun_attribute.hpp"
#include "stun/stun_parse_stat.hpp"
#include "stun/details/stun_attr_registry.hpp"
#include "stun/details/stun_fingerprint.hpp"

namespace freewebrtc::stun {

UnknownAttribute::UnknownAttribute(const util::ConstBinaryView& vv)
    : data(vv.begin(), vv.end())
{}


Attribute::Attribute(AttributeType t, Value&& v)
    : m_type(t)
    , m_value(std::move(v))
{}

std::optional<Attribute> Attribute::parse(const util::ConstBinaryView& vv, AttributeType type, ParseStat& stat) {
    switch (type.value()) {
        // case attr_registry::MAPPED_ADDRESS:
        //     if (auto maybe_attr = MappedAddressAttribute::parse(vv, stat); maybe_attr.has_value()) {
        //         return Attribute(type, *maybe_attr);
        //     }
        //     return std::nullopt;
        // case attr_registry::XOR_MAPPED_ADDRESS:
        //     if (auto maybe_attr = XorMappedAddressAttribute::parse(vv, stat); maybe_attr.has_value()) {
        //         return Attribute(type, *maybe_attr);
        //     }
        //     return std::nullopt;
        // case attr_registry::MESSAGE_INTEGRITY:
        //     if (auto maybe_attr = MessageIntegityAttribute::parse(vv, stat); maybe_attr.has_value()) {
        //         return Attribute(type, *maybe_attr);
        //     }
        //     return std::nullopt;
        case attr_registry::FINGERPRINT:
            if (auto maybe_attr = FingerprintAttribute::parse(vv, stat); maybe_attr.has_value()) {
                return Attribute(type, *maybe_attr);
            }
            return std::nullopt;
        default:
            return Attribute(type, UnknownAttribute(vv));
    }
}

std::optional<FingerprintAttribute> FingerprintAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    auto maybe_crc32 = vv.read_u32be(0);
    if (!maybe_crc32.has_value()) {
        stat.error.inc();
        stat.invalid_fingerprint_size.inc();
        return std::nullopt;
    }
    return FingerprintAttribute{*maybe_crc32 ^ FINGERPRINT_XOR};
}

}

