
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
#include "stun/details/stun_constants.hpp"

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
        case attr_registry::USERNAME:
            if (auto maybe_attr = UsernameAttribute::parse(vv, stat); maybe_attr.has_value()) {
                return Attribute(type, *maybe_attr);
            }
            return std::nullopt;
        case attr_registry::SOFTWARE:
            if (auto maybe_attr = SoftwareAttribute::parse(vv, stat); maybe_attr.has_value()) {
                return Attribute(type, *maybe_attr);
            }
            return std::nullopt;
        case attr_registry::MESSAGE_INTEGRITY:
            if (auto maybe_attr = MessageIntegityAttribute::parse(vv, stat); maybe_attr.has_value()) {
                return Attribute(type, *maybe_attr);
            }
            return std::nullopt;
        case attr_registry::FINGERPRINT:
            if (auto maybe_attr = FingerprintAttribute::parse(vv, stat); maybe_attr.has_value()) {
                return Attribute(type, *maybe_attr);
            }
            return std::nullopt;
        default:
            return Attribute(type, UnknownAttribute(vv));
    }
}

std::optional<XorMappedAddressAttribute> XorMappedAddressAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    //  0                   1                   2                   3
    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |0 0 0 0 0 0 0 0|    Family     |         X-Port                |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                X-Address (Variable)
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    auto maybe_family = vv.read_u8(1);
    auto maybe_xport = vv.read_u16be(2);
    const auto maybe_xaddr_view = vv.subview(4);
    if (!maybe_family.has_value() || !maybe_xport.has_value() || !maybe_xaddr_view.has_value()) {
        stat.error.inc();
        stat.invalid_xor_mapped_address.inc();
        return std::nullopt;
    }
    // X-Port is computed by XOR'ing the mapped port with the most
    // significant 16 bits of the magic cookie.
    auto port = net::Port(*maybe_xport ^ (details::MAGIC_COOKIE >> 16));
    switch (*maybe_family) {
        case attr_registry::FAMILY_IPV4: {
            auto maybe_xaddr = net::ip::AddressV4::from_view(*maybe_xaddr_view);
            if (!maybe_xaddr.has_value()) {
                stat.error.inc();
                stat.invalid_ip_address.inc();
                return std::nullopt;
            }
            return XorMappedAddressAttribute{*maybe_xaddr, port};
        }
        case attr_registry::FAMILY_IPV6: {
            auto maybe_xaddr = net::ip::AddressV6::from_view(*maybe_xaddr_view);
            if (!maybe_xaddr.has_value()) {
                stat.error.inc();
                stat.invalid_ip_address.inc();
                return std::nullopt;
            }
            return XorMappedAddressAttribute{*maybe_xaddr, port};
        }
    }
    return std::nullopt;
}


std::optional<SoftwareAttribute> SoftwareAttribute::parse(const util::ConstBinaryView& vv, ParseStat&) {
    return SoftwareAttribute{std::string(vv.begin(), vv.end())};
}

std::optional<UsernameAttribute> UsernameAttribute::parse(const util::ConstBinaryView& vv, ParseStat&) {
    return UsernameAttribute{precis::OpaqueString{std::string(vv.begin(), vv.end())}};
}

std::optional<MessageIntegityAttribute> MessageIntegityAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    auto maybe_digest = Digest::Value::from_view(vv);
    if (!maybe_digest.has_value()) {
        stat.error.inc();
        stat.invalid_message_integrity.inc();
        return std::nullopt;
    }
    return MessageIntegityAttribute{Digest(std::move(*maybe_digest))};
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

