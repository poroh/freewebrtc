//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Message functions
//

#include "stun/stun_message.hpp"
#include "stun/stun_error.hpp"
#include "stun/details/stun_attr_registry.hpp"
#include "stun/details/stun_fingerprint.hpp"
#include "stun/details/stun_constants.hpp"
#include "util/util_variant_overloaded.hpp"
#include "util/util_result.hpp"
#include "util/util_reduce.hpp"
#include <iostream>

namespace freewebrtc::stun {

class RawAttr {
public:
    static Result<RawAttr> parse(util::ConstBinaryView vv, size_t offset);
    uint16_t type() const noexcept;
    util::ConstBinaryView value() const noexcept;
    size_t aligned_length() const noexcept;

private:
    RawAttr(uint16_t type, uint16_t length, util::ConstBinaryView value);
    uint16_t m_type;
    uint16_t m_length;
    util::ConstBinaryView m_value;
};

struct ParseAttrsResult {
    std::vector<Attribute::ParseResult> attrs;
    Maybe<util::ConstBinaryView::Interval> maybe_integrity_interval;
    Maybe<util::ConstBinaryView::Interval> maybe_fingerprint_interval;
};

Result<ParseAttrsResult> parse_attrs(util::ConstBinaryView vv, size_t attr_offset, ParseStat& stat);

Result<Message> Message::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    using namespace details;
    if (vv.size() < STUN_HEADER_SIZE) {
        stat.error.inc();
        stat.invalid_size.inc();
        return make_error_code(ParseError::invalid_message_size);
    }
    //  0                   1                   2                   3
    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |0 0|     STUN Message Type     |         Message Length        |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                         Magic Cookie                          |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                                                               |
    // |                     Transaction ID (96 bits)                  |
    // |                                                               |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    const uint16_t msg_type = vv.assured_read_u16be(0);
    const uint16_t msg_length = vv.assured_read_u16be(2);
    const uint32_t magic_cookie = vv.assured_read_u32be(4);

    // The message length MUST contain the size, in bytes, of the message
    // not including the 20-byte STUN header.  Since all STUN attributes are
    // padded to a multiple of 4 bytes, the last 2 bits of this field are
    // always zero.  This provides another way to distinguish STUN packets
    // from packets of other protocols.
    if ((msg_length & 0x3) != 0) {
        stat.error.inc();
        stat.not_padded.inc();
        return make_error_code(ParseError::not_padded_attributes);
    }
    if (msg_length != vv.size() - STUN_HEADER_SIZE) {
        stat.error.inc();
        stat.message_length_error.inc();
        return make_error_code(ParseError::invalid_message_len);
    }

    const auto cls = Class::from_msg_type(msg_type);
    const IsRFC3489 is_rfc3489{cls.value() == Class::REQUEST && magic_cookie != details::MAGIC_COOKIE};
    if (!is_rfc3489 && magic_cookie != details::MAGIC_COOKIE) {
        stat.error.inc();
        stat.magic_cookie_error.inc();
        return make_error_code(ParseError::invalid_magic_cookie);
    }

    const auto transaction_id =
        !is_rfc3489 ? vv.assured_subview(8, TRANSACTION_ID_SIZE)
                    : vv.assured_subview(4, TRANSACTION_ID_SIZE_RFC3489);

    AttributeSet attrs;
    Maybe<util::ConstBinaryView::Interval> maybe_integrity_interval = none();
    Maybe<util::ConstBinaryView::Interval> maybe_fingerprint_interval = none();
    Maybe<uint32_t> fingerprint_value = none();
    return parse_attrs(vv, STUN_HEADER_SIZE, stat)
        .bind([&](ParseAttrsResult&& r) {
            maybe_integrity_interval = r.maybe_integrity_interval;
            maybe_fingerprint_interval = r.maybe_fingerprint_interval;
            return util::reduce(r.attrs.begin(), r.attrs.end(), [&](auto&& pr) {
                return std::visit(
                    util::overloaded {
                        [&](UnknownAttribute&& attr) {
                            attrs.emplace(std::move(attr));
                            return success();
                        },
                        [&](Attribute&& attr) -> MaybeError {
                            if (const auto *fingerprint = attr.as<FingerprintAttribute>(); fingerprint != nullptr) {
                                fingerprint_value = fingerprint->crc32;
                            }
                            attrs.emplace(std::move(attr));
                            return success();
                        }
                 }, std::move(pr));
            });
        })
        .bind([&](auto&&) -> MaybeError {
            // check fingerprint if among attributes
            auto fingerprint_is_valid = fingerprint_value
                .fmap([&](auto fp) {
                    return maybe_fingerprint_interval
                        .bind([&](auto interval) {
                            return vv.subview(interval);
                        })
                        .fmap([&](auto v) {
                            // Normative:
                            // The value of the attribute is computed as the CRC-32 of the STUN message
                            // up to (but excluding) the FINGERPRINT attribute itself, XOR'ed with
                            // the 32-bit value 0x5354554e (the XOR helps in cases where an
                            // application packet is also using CRC-32 in it).
                            //
                            // Implementation: we xored fingerprint with 0x5354554e in FingerprintAttribute so
                            // it fingerprint->crc32 must be equal to crc32 of the message without additional xor.
                            return crc32(v) == fp;
                        }).value_or(false);
                })
                .value_or(true);
            if (!fingerprint_is_valid) {
                stat.error.inc();
                stat.invalid_fingerprint.inc();
                return make_error_code(ParseError::fingerprint_not_valid);
            }
            return success();
        })
        .fmap([&](auto&&) {
            stat.success.inc();
            return Message {
                Header {
                    cls,
                    Method::from_msg_type(msg_type),
                    TransactionId(transaction_id)
                },
                std::move(attrs),
                is_rfc3489,
                maybe_integrity_interval
            };
        });
}

Result<Maybe<bool>> Message::is_valid(const util::ConstBinaryView& data, const IntegrityData& idata) const noexcept {
    using namespace details;
    using MaybeBool = Maybe<bool>;
    using View = util::ConstBinaryView;
    using DigestRef = std::reference_wrapper<const MessageIntegityAttribute::Digest>;
    struct Data {
        DigestRef digest;
        View without_4byte_header;
        size_t integrity_message_len;
    };
    size_t integrity_message_len = 0;
    return integrity_interval
        .bind([&](auto&& ii) {
            return data.subview(ii);
        })
        .bind([&](util::ConstBinaryView&& c) {
            integrity_message_len = c.size() + STUN_ATTR_HEADER_SIZE + crypto::SHA1Hash::size - STUN_HEADER_SIZE;
            return c.subview(4);
        })
        .bind([&](auto&& h) {
            return attribute_set.integrity().fmap([&](const DigestRef& digest) {
                return Data{
                    .digest = digest,
                    .without_4byte_header = h,
                    .integrity_message_len = integrity_message_len
                };
            });
        })
        .fmap([&](const Data& d) -> Result<MaybeBool> {
            // Fake header for integrity checking:
            std::array<uint8_t, 4> header = {
                data.data()[0],
                data.data()[1],
                uint8_t((d.integrity_message_len >> 8) & 0xFF),
                uint8_t(d.integrity_message_len & 0xFF)
            };
            const auto& p = idata.password;
            return crypto::hmac::digest({View(header), d.without_4byte_header}, p.opad(), p.ipad(), idata.hash)
                .fmap([&](auto&& digest) {
                    return MaybeBool{digest.value == d.digest.get().value};
                });
        })
        .value_or(Result<MaybeBool>{none()});
}


Result<util::ByteVec> Message::build(const MaybeIntegrity& maybeintegrity) const noexcept {
    return attribute_set.build(header, maybeintegrity);
}

RawAttr::RawAttr(uint16_t type, uint16_t length, util::ConstBinaryView value)
    : m_type(type)
    , m_length(length)
    , m_value(value)
{
}

uint16_t RawAttr::type() const noexcept {
    return m_type;
}

util::ConstBinaryView RawAttr::value() const noexcept {
    return m_value;
}

Result<RawAttr> RawAttr::parse(util::ConstBinaryView vv, size_t offset) {
    // 0                   1                   2                   3
    // 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |         Type                  |            Length             |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                         Value (variable)                ....
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    const auto type_rv = vv.read_u16be(offset).require();
    const auto length_rv = vv.read_u16be(offset + 2).require();
    // The value in the length field MUST contain the length of the Value
    // part of the attribute, prior to padding, measured in bytes.
    const auto attr_view_rv =
        length_rv.bind([&](auto&& length) {
            return vv.subview(offset + sizeof(uint32_t), length).require();
        });
    return combine([](uint16_t type, uint16_t length, util::ConstBinaryView value) -> Result<RawAttr> {
        return RawAttr(type, length, value);
    }, type_rv, length_rv, attr_view_rv);
}

size_t RawAttr::aligned_length() const noexcept {
    using namespace details;
    // Since STUN aligns attributes on 32-bit boundaries, attributes whose content
    // is not a multiple of 4 bytes are padded with 1, 2, or 3 bytes of
    // padding so that its value contains a multiple of 4 bytes.  The
    // padding bits are ignored, and may be any value.
    const auto align = m_length % sizeof(uint32_t) == 0 ? 0 : 1;
    const auto align_length = (m_length / sizeof(uint32_t) + align) * sizeof(uint32_t);
    return align_length + STUN_ATTR_HEADER_SIZE;
}

Result<ParseAttrsResult> parse_attrs(util::ConstBinaryView vv, size_t attr_offset, ParseStat& stat) {
    std::vector<RawAttr> raw_attrs;
    Maybe<util::ConstBinaryView::Interval> maybe_integrity_interval = none();
    Maybe<util::ConstBinaryView::Interval> maybe_fingerprint_interval = none();
    while (attr_offset < vv.size()) {
        auto rawattr_rv = RawAttr::parse(vv, attr_offset);
        if (rawattr_rv.is_err()) {
            stat.error.inc();
            stat.invalid_attr_size.inc();
            return make_error_code(ParseError::invalid_attr_size);
        }
        const auto raw_attr = rawattr_rv.unwrap();
        switch (raw_attr.type()) {
        case attr_registry::FINGERPRINT:
            raw_attrs.emplace_back(raw_attr);
            maybe_fingerprint_interval = util::ConstBinaryView::Interval{0, attr_offset};
            // 15.5.  FINGERPRINT
            // When present, the FINGERPRINT attribute MUST be the last attribute in
            // the message, and thus will appear after MESSAGE-INTEGRITY.
            if (attr_offset + raw_attr.aligned_length() < vv.size()) {
                stat.error.inc();
                stat.fingerprint_not_last.inc();
                return make_error_code(ParseError::fingerprint_is_not_last);
            }
            break;
        case attr_registry::MESSAGE_INTEGRITY:
            // 15.4.  MESSAGE-INTEGRITY
            // The text used as input to HMAC is the STUN message,
            // including the header, up to and including the attribute
            // preceding the MESSAGE-INTEGRITY attribute.
            raw_attrs.emplace_back(raw_attr);
            maybe_integrity_interval = util::ConstBinaryView::Interval{0, attr_offset};
            break;
        default:
            if (!maybe_integrity_interval.is_some()) {
                // RFC5389:
                // 15.4.  MESSAGE-INTEGRITY
                // With the exception of the FINGERPRINT attribute,
                // which appears after MESSAGE-INTEGRITY, agents MUST
                // ignore all other attributes that follow
                // MESSAGE-INTEGRITY.
                raw_attrs.emplace_back(raw_attr);
            }
            break;
        }
        attr_offset += raw_attr.aligned_length();
    }
    using Result = std::vector<Attribute::ParseResult>;
    Result result;
    result.reserve(raw_attrs.size());
    return util::reduce(raw_attrs.begin(), raw_attrs.end(), result, [&](Result&& result, const RawAttr& raw_attr) {
        const auto attr_view = raw_attr.value();
        const auto attr_type = AttributeType::from_uint16(raw_attr.type());
        return Attribute::parse(attr_view, attr_type, stat)
            .fmap([&](Attribute::ParseResult&& attr) {
                result.emplace_back(std::move(attr));
                return result;
            });
    }).fmap([&](Result&& attrs) {
        return ParseAttrsResult {
            .attrs = std::move(attrs),
            .maybe_integrity_interval = maybe_integrity_interval,
            .maybe_fingerprint_interval = maybe_fingerprint_interval,
        };
    });
}

}
