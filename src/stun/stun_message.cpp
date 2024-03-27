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
#include <iostream>

namespace freewebrtc::stun {


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
    std::optional<util::ConstBinaryView::Interval> integrity_interval;
    size_t attr_offset = STUN_HEADER_SIZE;
    while (attr_offset < vv.size()) {
        // 0                   1                   2                   3
        // 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |         Type                  |            Length             |
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |                         Value (variable)                ....
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        const auto maybe_type = vv.read_u16be(attr_offset);
        const auto maybe_length = vv.read_u16be(attr_offset + 2);
        // The value in the length field MUST contain the length of the Value
        // part of the attribute, prior to padding, measured in bytes.
        const auto length = maybe_length.value_or(0);
        const auto maybe_attr_view = vv.subview(attr_offset + sizeof(uint32_t), length);
        if (!maybe_type.has_value() || !maybe_length.has_value() || !maybe_attr_view.has_value()) {
            stat.error.inc();
            stat.invalid_attr_size.inc();
            return make_error_code(ParseError::invalid_attr_size);
        }
        const auto type = *maybe_type;
        const auto attr_view = *maybe_attr_view;
        // Since STUN aligns attributes on 32-bit boundaries, attributes whose content
        // is not a multiple of 4 bytes are padded with 1, 2, or 3 bytes of
        // padding so that its value contains a multiple of 4 bytes.  The
        // padding bits are ignored, and may be any value.
        const auto align = length % sizeof(uint32_t) == 0 ? 0 : 1;
        const auto align_length = (length / sizeof(uint32_t) + align) * sizeof(uint32_t);

        const auto attr_type = AttributeType::from_uint16(type);
        const auto next_attr_offset = attr_offset + align_length + STUN_ATTR_HEADER_SIZE;
        // RFC5389:
        // 15.4.  MESSAGE-INTEGRITY
        // With the exception of the FINGERPRINT attribute, which
        // appears after MESSAGE-INTEGRITY, agents MUST ignore all
        // other attributes that follow MESSAGE-INTEGRITY.
        if (integrity_interval.has_value() && type != attr_registry::FINGERPRINT) {
            attr_offset = next_attr_offset;
            continue;
        }
        auto attr_rv = Attribute::parse(attr_view, attr_type, stat);
        if (attr_rv.is_err()) {
            // statisitics is increased by Attribute::parse.
            return attr_rv.unwrap_err();
        }
        const auto maybe_error =
            std::visit(
                util::overloaded {
                    [&](UnknownAttribute&& attr) {
                        attrs.emplace(std::move(attr));
                        return success();
                    },
                    [&](Attribute&& attr) -> MaybeError {
                        if (attr.as<MessageIntegityAttribute>() != nullptr) {
                            // 15.4.  MESSAGE-INTEGRITY
                            // The text used as input to HMAC is the STUN message,
                            // including the header, up to and including the
                            // attribute preceding the MESSAGE-INTEGRITY
                            // attribute.
                            integrity_interval = util::ConstBinaryView::Interval{0, attr_offset};
                        }
                        if (const auto *fingerprint = attr.as<FingerprintAttribute>(); fingerprint != nullptr) {
                            // 15.5.  FINGERPRINT
                            // When present, the FINGERPRINT attribute MUST be the last attribute in
                            // the message, and thus will appear after MESSAGE-INTEGRITY.
                            if (next_attr_offset < vv.size()) {
                                stat.error.inc();
                                stat.fingerprint_not_last.inc();
                                return make_error_code(ParseError::fingerprint_is_not_last);
                            }
                            // Normative:
                            // The value of the attribute is computed as the CRC-32 of the STUN message
                            // up to (but excluding) the FINGERPRINT attribute itself, XOR'ed with
                            // the 32-bit value 0x5354554e (the XOR helps in cases where an
                            // application packet is also using CRC-32 in it).
                            //
                            // Implementation: we xored fingerprint with 0x5354554e in FingerprintAttribute so
                            // it fingerprint->crc32 must be equal to crc32 of the message without additional xor.
                            const uint32_t v = crc32(vv.assured_subview(0, attr_offset));
                            if (v != fingerprint->crc32) {
                                stat.error.inc();
                                stat.invalid_fingerprint.inc();
                                return make_error_code(ParseError::fingerprint_not_valid);
                            }
                        }
                        attrs.emplace(std::move(attr));
                        return success();
                    }
                },
                std::move(attr_rv.unwrap()));
        if (maybe_error.is_err()) {
            return maybe_error.unwrap_err();
        }
        attr_offset = next_attr_offset;
    }

    stat.success.inc();
    return Message {
        Header {
            cls,
            Method::from_msg_type(msg_type),
            TransactionId(transaction_id)
         },
         std::move(attrs),
         is_rfc3489,
         integrity_interval
    };
}

Result<std::optional<bool>> Message::is_valid(const util::ConstBinaryView& data, const IntegrityData& idata) const noexcept {
    using namespace details;
    const auto& h = idata.hash;
    const auto& password = idata.password;
    using MaybeBool = std::optional<bool>;
    if (!integrity_interval.has_value()) {
        return MaybeBool{std::nullopt};
    }
    const auto maybe_covered = data.subview(*integrity_interval);
    if (!maybe_covered.has_value()) {
        return MaybeBool{std::nullopt};
    }
    const auto maybe_integrity = attribute_set.integrity();
    if (!maybe_integrity.has_value()) {
        return MaybeBool{std::nullopt};
    }
    const auto& integrity = *maybe_integrity;
    const auto& covered = *maybe_covered;
    const auto without_4byte_header = covered.subview(4);
    if (!without_4byte_header.has_value()) {
        return MaybeBool{std::nullopt};
    }
    const size_t integrity_message_len = covered.size() + STUN_ATTR_HEADER_SIZE + crypto::SHA1Hash::size - STUN_HEADER_SIZE;
    // Fake header for integrity checking:
    std::array<uint8_t, 4> header =
        {
            data.data()[0],
            data.data()[1],
            uint8_t((integrity_message_len >> 8) & 0xFF),
            uint8_t(integrity_message_len & 0xFF)
        };
    return crypto::hmac::digest({util::ConstBinaryView(header), *without_4byte_header}, password.opad(), password.ipad(), h)
        .fmap([&](auto&& digest) {
            return MaybeBool{digest.value == integrity.get().value};
        });
}

Result<util::ByteVec> Message::build(const MaybeIntegrity& maybeintegrity) const noexcept {
    return attribute_set.build(header, maybeintegrity);
}

}
