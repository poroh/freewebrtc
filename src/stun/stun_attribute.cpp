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
#include "util/util_fmap.hpp"

namespace freewebrtc::stun {

UnknownAttribute::UnknownAttribute(const util::ConstBinaryView& vv)
    : data(vv.begin(), vv.end())
{}

Attribute::Attribute(AttributeType t, Value&& v)
    : m_type(t)
    , m_value(std::move(v))
{}

std::optional<Attribute> Attribute::parse(const util::ConstBinaryView& vv, AttributeType type, ParseStat& stat) {
    auto create_attr_fun = [=](auto&& attr) { return Attribute(type, attr); };
    switch (type.value()) {
    // case attr_registry::MAPPED_ADDRESS:  return util::fmap(MappedAddressAttribute::parse(vv, stat),    std::move(create_attr_fun));
    case attr_registry::XOR_MAPPED_ADDRESS: return util::fmap(XorMappedAddressAttribute::parse(vv, stat), std::move(create_attr_fun));
    case attr_registry::USERNAME:           return util::fmap(UsernameAttribute::parse(vv, stat),         std::move(create_attr_fun));
    case attr_registry::SOFTWARE:           return util::fmap(SoftwareAttribute::parse(vv, stat),         std::move(create_attr_fun));
    case attr_registry::MESSAGE_INTEGRITY:  return util::fmap(MessageIntegityAttribute::parse(vv, stat),  std::move(create_attr_fun));
    case attr_registry::FINGERPRINT:        return util::fmap(FingerprintAttribute::parse(vv, stat),      std::move(create_attr_fun));
    case attr_registry::PRIORITY:           return util::fmap(PriorityAttribute::parse(vv, stat),         std::move(create_attr_fun));
    case attr_registry::ICE_CONTROLLING:    return util::fmap(IceControllingAttribute::parse(vv, stat),   std::move(create_attr_fun));
    case attr_registry::ICE_CONTROLLED:     return util::fmap(IceControlledAttribute::parse(vv, stat),   std::move(create_attr_fun));
    case attr_registry::USE_CANDIDATE:      return util::fmap(UseCandidateAttribute::parse(vv, stat),     std::move(create_attr_fun));
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
    auto maybe_family = Family::from_uint8(vv.read_u8(1));
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
    auto maybe_xaddr = XoredAddress::from_view(*maybe_family, *maybe_xaddr_view);
    if (!maybe_xaddr.has_value()) {
        stat.error.inc();
        stat.invalid_ip_address.inc();
        return std::nullopt;
    }
    return XorMappedAddressAttribute{*maybe_xaddr, port};
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

std::optional<PriorityAttribute> PriorityAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    auto maybe_priority = vv.read_u32be(0);
    if (!maybe_priority.has_value()) {
        stat.error.inc();
        stat.invalid_priority_size.inc();
        return std::nullopt;
    }
    return PriorityAttribute{*maybe_priority};
}

std::optional<IceControllingAttribute> IceControllingAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    auto maybe_tiebreaker = vv.read_u64be(0);
    if (!maybe_tiebreaker.has_value()) {
        stat.error.inc();
        stat.invalid_ice_controlling_size.inc();
        return std::nullopt;
    }
    return IceControllingAttribute{*maybe_tiebreaker};
}

std::optional<IceControlledAttribute> IceControlledAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    auto maybe_tiebreaker = vv.read_u64be(0);
    if (!maybe_tiebreaker.has_value()) {
        stat.error.inc();
        stat.invalid_ice_controlled_size.inc();
        return std::nullopt;
    }
    return IceControlledAttribute{*maybe_tiebreaker};
}

std::optional<UseCandidateAttribute> UseCandidateAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    // The USE-CANDIDATE attribute indicates that the candidate pair
    // resulting from this check will be used for transmission of data.  The
    // attribute has no content (the Length field of the attribute is zero);
    // it serves as a flag.
    if (vv.size() != 0) {
        stat.error.inc();
        stat.invalid_use_candidate_size.inc();
        return std::nullopt;
    }
    return UseCandidateAttribute{};
}

}

