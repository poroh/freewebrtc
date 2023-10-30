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
#include "util/util_variant_overloaded.hpp"

namespace freewebrtc::stun {

UnknownAttribute::UnknownAttribute(AttributeType t, const util::ConstBinaryView& vv)
    : type(t)
    , data(vv.begin(), vv.end())
{}

Attribute::Attribute(AttributeType t, Value&& v)
    : m_type(t)
    , m_value(std::move(v))
{}

std::optional<Attribute::ParseResult> Attribute::parse(const util::ConstBinaryView& vv, AttributeType type, ParseStat& stat) {
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
    case attr_registry::ICE_CONTROLLED:     return util::fmap(IceControlledAttribute::parse(vv, stat),    std::move(create_attr_fun));
    case attr_registry::USE_CANDIDATE:      return util::fmap(UseCandidateAttribute::parse(vv, stat),     std::move(create_attr_fun));
    default:
        return UnknownAttribute(type, vv);
    }
}

Attribute Attribute::create(Value&& v) {
    return std::visit(
        util::overloaded {
            [](XorMappedAddressAttribute&& a)  { return Attribute(AttributeType::from_uint16(attr_registry::XOR_MAPPED_ADDRESS), std::move(a)); },
            [](UsernameAttribute&& a)          { return Attribute(AttributeType::from_uint16(attr_registry::USERNAME), std::move(a)); },
            [](SoftwareAttribute&& a)          { return Attribute(AttributeType::from_uint16(attr_registry::SOFTWARE), std::move(a)); },
            [](MessageIntegityAttribute&& a)   { return Attribute(AttributeType::from_uint16(attr_registry::MESSAGE_INTEGRITY), std::move(a)); },
            [](FingerprintAttribute&& a)       { return Attribute(AttributeType::from_uint16(attr_registry::FINGERPRINT), std::move(a)); },
            [](PriorityAttribute&& a)          { return Attribute(AttributeType::from_uint16(attr_registry::PRIORITY), std::move(a)); },
            [](IceControllingAttribute&& a)    { return Attribute(AttributeType::from_uint16(attr_registry::ICE_CONTROLLING), std::move(a)); },
            [](IceControlledAttribute&& a)     { return Attribute(AttributeType::from_uint16(attr_registry::ICE_CONTROLLED), std::move(a)); },
            [](UseCandidateAttribute&& a)      { return Attribute(AttributeType::from_uint16(attr_registry::USE_CANDIDATE), std::move(a)); },
            [](UnknownAttributesAttribute&& a) { return Attribute(AttributeType::from_uint16(attr_registry::UNKNOWN_ATTRIBUTES), std::move(a)); },
            [](ErrorCodeAttribute&& a)         { return Attribute(AttributeType::from_uint16(attr_registry::ERROR_CODE), std::move(a)); }
        },
        std::move(v));
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

util::ByteVec XorMappedAddressAttribute::build() const {
    const uint16_t xport = port.value() ^ (details::MAGIC_COOKIE >> 16);
    const uint8_t family = addr.family().to_uint8();
    const uint32_t first_word = util::host_to_network_u32(xport | (family >> 16));
    return util::ConstBinaryView::concat({
            util::ConstBinaryView(&first_word, sizeof(first_word)),
            addr.view()
        });
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

util::ByteVec UnknownAttributesAttribute::build() const {
    //  0                   1                   2                   3
    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |      Attribute 1 Type           |     Attribute 2 Type        |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |      Attribute 3 Type           |     Attribute 4 Type    ...
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    using AttrRawType = uint16_t;
    std::vector<uint8_t> result(types.size() * sizeof(AttrRawType));
    auto pos = result.begin();
    for (auto t: types) {
        AttrRawType tv = util::host_to_network_u16(t.value());
        memcpy(&*pos, &tv, sizeof(AttrRawType));
        pos += sizeof(tv);
    }
    return result;
}

util::ByteVec ErrorCodeAttribute::build() const {
    //  0                   1                   2                   3
    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |           Reserved, should be 0         |Class|     Number    |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |      Reason Phrase (variable)                                ..
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    uint32_t first_word = util::host_to_network_u32(((code / 100) << 5) | (code % 100));
    return util::ConstBinaryView::concat({
            util::ConstBinaryView(&first_word, sizeof(first_word)),
            reason_phrase.has_value() ? util::ConstBinaryView(reason_phrase->data(), reason_phrase->size())
                                      : util::ConstBinaryView(&first_word, 0) // 0 is by intention!
        });
}

}

