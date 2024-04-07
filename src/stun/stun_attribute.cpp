//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Attribute
//

#include "stun/stun_attribute.hpp"
#include "stun/stun_error.hpp"
#include "stun/stun_parse_stat.hpp"
#include "stun/details/stun_attr_registry.hpp"
#include "stun/details/stun_fingerprint.hpp"
#include "stun/details/stun_constants.hpp"
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

Result<Attribute::ParseResult> Attribute::parse(const util::ConstBinaryView& vv, AttributeType type, ParseStat& stat) {
    auto create_attr_fun = [=](Value&& attr) -> Result<ParseResult> { return ParseResult{Attribute(type, std::move(attr))}; };
    switch (type.value()) {
    case attr_registry::MAPPED_ADDRESS:     return MappedAddressAttribute::parse(vv, stat).bind(std::move(create_attr_fun));
    case attr_registry::XOR_MAPPED_ADDRESS: return XorMappedAddressAttribute::parse(vv, stat).bind(std::move(create_attr_fun));
    case attr_registry::USERNAME:           return UsernameAttribute::parse(vv, stat).bind(std::move(create_attr_fun));
    case attr_registry::SOFTWARE:           return SoftwareAttribute::parse(vv, stat).bind(std::move(create_attr_fun));
    case attr_registry::MESSAGE_INTEGRITY:  return MessageIntegityAttribute::parse(vv, stat).bind(std::move(create_attr_fun));
    case attr_registry::FINGERPRINT:        return FingerprintAttribute::parse(vv, stat).bind(std::move(create_attr_fun));
    case attr_registry::PRIORITY:           return PriorityAttribute::parse(vv, stat).bind(std::move(create_attr_fun));
    case attr_registry::ICE_CONTROLLING:    return IceControllingAttribute::parse(vv, stat).bind(std::move(create_attr_fun));
    case attr_registry::ICE_CONTROLLED:     return IceControlledAttribute::parse(vv, stat).bind(std::move(create_attr_fun));
    case attr_registry::USE_CANDIDATE:      return UseCandidateAttribute::parse(vv, stat).bind(std::move(create_attr_fun));
    case attr_registry::ERROR_CODE:         return ErrorCodeAttribute::parse(vv, stat).bind(std::move(create_attr_fun));
    case attr_registry::ALTERNATE_SERVER:   return AlternateServerAttribute::parse(vv, stat).bind(std::move(create_attr_fun));
    case attr_registry::UNKNOWN_ATTRIBUTES: return UnknownAttributesAttribute::parse(vv, stat).bind(std::move(create_attr_fun));
    default:
        return ParseResult{UnknownAttribute(type, vv)};
    }
}

Attribute Attribute::create(Value&& v) {
    return std::visit(
        util::overloaded {
            [](XorMappedAddressAttribute&& a)  { return Attribute(AttributeType::from_uint16(attr_registry::XOR_MAPPED_ADDRESS), std::move(a)); },
            [](MappedAddressAttribute&& a)     { return Attribute(AttributeType::from_uint16(attr_registry::MAPPED_ADDRESS), std::move(a)); },
            [](UsernameAttribute&& a)          { return Attribute(AttributeType::from_uint16(attr_registry::USERNAME), std::move(a)); },
            [](SoftwareAttribute&& a)          { return Attribute(AttributeType::from_uint16(attr_registry::SOFTWARE), std::move(a)); },
            [](MessageIntegityAttribute&& a)   { return Attribute(AttributeType::from_uint16(attr_registry::MESSAGE_INTEGRITY), std::move(a)); },
            [](FingerprintAttribute&& a)       { return Attribute(AttributeType::from_uint16(attr_registry::FINGERPRINT), std::move(a)); },
            [](PriorityAttribute&& a)          { return Attribute(AttributeType::from_uint16(attr_registry::PRIORITY), std::move(a)); },
            [](IceControllingAttribute&& a)    { return Attribute(AttributeType::from_uint16(attr_registry::ICE_CONTROLLING), std::move(a)); },
            [](IceControlledAttribute&& a)     { return Attribute(AttributeType::from_uint16(attr_registry::ICE_CONTROLLED), std::move(a)); },
            [](UseCandidateAttribute&& a)      { return Attribute(AttributeType::from_uint16(attr_registry::USE_CANDIDATE), std::move(a)); },
            [](UnknownAttributesAttribute&& a) { return Attribute(AttributeType::from_uint16(attr_registry::UNKNOWN_ATTRIBUTES), std::move(a)); },
            [](ErrorCodeAttribute&& a)         { return Attribute(AttributeType::from_uint16(attr_registry::ERROR_CODE), std::move(a)); },
            [](AlternateServerAttribute&& a)   { return Attribute(AttributeType::from_uint16(attr_registry::ALTERNATE_SERVER), std::move(a)); }
        },
        std::move(v));
}

Result<MappedAddressAttribute> MappedAddressAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    //  0                   1                   2                   3
    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |x x x x x x x x|    Family     |           Port                |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                             Address                           |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //
    auto family_rv = vv.read_u8(1).require();
    auto port_rv = vv.read_u16be(2).require()
        .fmap([](auto& v) { return net::Port(v); });
    const auto addr_view_rv = vv.subview(4).require();
    return combine(
        [&](auto&& family, auto&& port, auto&& addr_view) {
            return
                ([&]() -> Result<net::ip::Address> {
                    switch (family) {
                    case attr_registry::FAMILY_IPV4:
                        return net::ip::AddressV4::from_view(addr_view)
                            .fmap([](auto&& addr) { return net::ip::Address(std::move(addr)); });
                    case attr_registry::FAMILY_IPV6:
                        return net::ip::AddressV6::from_view(addr_view)
                            .fmap([](auto&& addr) { return net::ip::Address(std::move(addr)); });
                    }
                    return make_error_code(ParseError::unknown_addr_family);
                })()
                .bind_err([&](auto&& err) {
                    stat.error.inc();
                    stat.invalid_ip_address.inc();
                    return err;
                })
                .fmap([&](auto&& addr) {
                    return MappedAddressAttribute{std::move(addr), port};
                });
        },
        std::move(family_rv),
        std::move(port_rv),
        std::move(addr_view_rv));
}

util::ByteVec MappedAddressAttribute::build() const {
    auto [family, view]
        = std::visit(
            util::overloaded {
                [](const net::ip::AddressV4& a) {
                    return std::make_pair(attr_registry::FAMILY_IPV4, a.view());
                },
                [](const net::ip::AddressV6& a) {
                    return std::make_pair(attr_registry::FAMILY_IPV6, a.view());
                }
            }, addr.value());
    const uint32_t first_word = util::host_to_network_u32(port.value() | (family >> 16));
    return util::ConstBinaryView::concat({
            util::ConstBinaryView(&first_word, sizeof(first_word)),
            view
        });
}

Result<XorMappedAddressAttribute> XorMappedAddressAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    //  0                   1                   2                   3
    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |0 0 0 0 0 0 0 0|    Family     |         X-Port                |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                X-Address (Variable)
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    const auto family_rv = vv.read_u8(1).require().bind(Family::from_uint8);
    const auto port_rv = vv.read_u16be(2).require()
        .bind_err([&](auto&& err) {
            stat.error.inc();
            stat.invalid_xor_mapped_address.inc();
            return err;
        })
        .fmap([](auto xport) {
            // X-Port is computed by XOR'ing the mapped port with the most
            // significant 16 bits of the magic cookie.
            return net::Port(xport ^ (details::MAGIC_COOKIE >> 16));
        });
    if (port_rv.is_err()) {
        return port_rv.unwrap_err();
    }
    const auto xaddr_view_rv = vv.subview(4).require();
    const auto xaddr_rv =
        combine([&](auto&& family, auto&& xaddr_view) {
            return XoredAddress::from_view(family, xaddr_view);
        }, std::move(family_rv), std::move(xaddr_view_rv))
        .bind_err([&](auto&& err) {
            stat.error.inc();
            stat.invalid_ip_address.inc();
            return err;
        });
    return
        combine([&](auto&& xaddr, auto&& port) {
            return success(XorMappedAddressAttribute{std::move(xaddr), port});
        }, std::move(xaddr_rv), std::move(port_rv));
}

util::ByteVec XorMappedAddressAttribute::build() const {
    const uint16_t xport = port.value() ^ (details::MAGIC_COOKIE >> 16);
    const uint8_t family = addr.family().to_uint8();
    const uint32_t first_word = util::host_to_network_u32(xport | (family << 16));
    return util::ConstBinaryView::concat({
            util::ConstBinaryView(&first_word, sizeof(first_word)),
            addr.view()
        });
}

Result<SoftwareAttribute> SoftwareAttribute::parse(const util::ConstBinaryView& vv, ParseStat&) {
    return SoftwareAttribute{std::string(vv.begin(), vv.end())};
}

Result<UsernameAttribute> UsernameAttribute::parse(const util::ConstBinaryView& vv, ParseStat&) {
    return UsernameAttribute{precis::OpaqueString{std::string(vv.begin(), vv.end())}};
}

Result<MessageIntegityAttribute> MessageIntegityAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    return Digest::Value::from_view(vv)
        .require()
        .bind_err([&](auto&&) {
            stat.error.inc();
            stat.invalid_message_integrity.inc();
            return make_error_code(ParseError::integrity_digest_size);
        })
        .fmap([](auto&& v) {
            return MessageIntegityAttribute{Digest(std::move(v))};
        });
}

Result<FingerprintAttribute> FingerprintAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    return vv.read_u32be(0)
        .require()
        .bind_err([&](auto&&) {
            stat.error.inc();
            stat.invalid_fingerprint_size.inc();
            return make_error_code(ParseError::fingerprint_crc_size);
        })
        .fmap([](auto&& crc32) {
            return FingerprintAttribute{crc32 ^ FINGERPRINT_XOR};
        });
}

Result<PriorityAttribute> PriorityAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    return vv.read_u32be(0)
        .require()
        .bind_err([&](auto&&) {
            stat.error.inc();
            stat.invalid_priority_size.inc();
            return make_error_code(ParseError::priority_attribute_size);
        })
        .fmap([](auto&& priority) {
            return PriorityAttribute{priority};
        });
}

Result<IceControllingAttribute> IceControllingAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    return vv.read_u64be(0)
        .require()
        .bind_err([&](auto&&) {
            stat.error.inc();
            stat.invalid_ice_controlling_size.inc();
            return make_error_code(ParseError::ice_controlling_size);
        })
        .fmap([](auto&& tiebreaker) {
            return IceControllingAttribute{tiebreaker};
        });
}

Result<IceControlledAttribute> IceControlledAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    return vv.read_u64be(0)
        .require()
        .bind_err([&](auto&&) {
            stat.error.inc();
            stat.invalid_ice_controlled_size.inc();
            return make_error_code(ParseError::ice_controlled_size);
        })
        .fmap([](auto&& tiebreaker) {
            return IceControlledAttribute{tiebreaker};
        });
}

Result<UseCandidateAttribute> UseCandidateAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    // The USE-CANDIDATE attribute indicates that the candidate pair
    // resulting from this check will be used for transmission of data.  The
    // attribute has no content (the Length field of the attribute is zero);
    // it serves as a flag.
    if (vv.size() != 0) {
        stat.error.inc();
        stat.invalid_use_candidate_size.inc();
        return make_error_code(ParseError::use_candidate_size);
    }
    return UseCandidateAttribute{};
}

Result<UnknownAttributesAttribute> UnknownAttributesAttribute::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    if (vv.size() % 2 != 0) {
        stat.error.inc();
        stat.invalid_unknown_attributes_attr_size.inc();
        return make_error_code(ParseError::unknown_attributes_attribute_size);
    }
    unsigned num = vv.size() / 2;
    UnknownAttributesAttribute attr;
    attr.types.reserve(num);
    for (unsigned i = 0; i < num; ++i) {
        attr.types.emplace_back(AttributeType::from_uint16(vv.assured_read_u16be(i * 2)));
    }
    return std::move(attr);
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
    uint32_t first_word = util::host_to_network_u32(((code / 100) << 8) | (code % 100));
    return util::ConstBinaryView::concat({
        util::ConstBinaryView(&first_word, sizeof(first_word)),
        reason_phrase
            .fmap([](const auto& reason_phrase) {
                return util::ConstBinaryView(
                    reason_phrase.data(),
                    reason_phrase.size());
            })
            .value_or(util::ConstBinaryView(&first_word, 0)) // 0 is by intention!
    });
}

Result<ErrorCodeAttribute> ErrorCodeAttribute::parse(const util::ConstBinaryView& v, ParseStat& stat) {
    auto maybe_reason = v.subview(4);
    return v.read_u32be(0)
        .require()
        .bind_err([&](auto&&) {
            stat.error.inc();
            stat.invalid_error_code_size.inc();
            return make_error_code(ParseError::error_code_attribute_size);
        })
        .fmap([&](uint32_t first_word) {
            return ErrorCodeAttribute{
                .code = (first_word >> 8) * 100 + (first_word & 0xFF),
                .reason_phrase = maybe_reason.fmap(
                    [](auto&& view) {
                        return std::string(view.begin(), view.end());
                    })};
        });
}

bool ErrorCodeAttribute::operator==(const ErrorCodeAttribute& other) const noexcept {
    return this->code == other.code;
}


Result<AlternateServerAttribute> AlternateServerAttribute::parse(const util::ConstBinaryView& view, ParseStat& stat) {
    // It is encoded in the same way as MAPPED-ADDRESS, and thus refers to a
    // single server by IP address.  The IP address family MUST be identical
    // to that of the source IP address of the request.
    //
    // TODO: It can be improved from error codes & statistics perspectice
    // by creating common function that parses MAPPED-ADDRESS for this attribute
    // and MAPPED-ADDRESS itself.
    return MappedAddressAttribute::parse(view, stat)
        .fmap([](MappedAddressAttribute&& attr) {
            return AlternateServerAttribute{std::move(attr.addr), std::move(attr.port)};
        });
}

util::ByteVec AlternateServerAttribute::build() const {
    MappedAddressAttribute mapped{addr, port};
    return mapped.build();
}


}

