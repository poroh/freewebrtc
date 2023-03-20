//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Message functions
//

#include "stun/stun_message.hpp"
#include "stun/details/stun_attr_registry.hpp"

namespace freewebrtc::stun {

// RFC5389: All STUN messages MUST start with a 20-byte header followed by zero
// or more Attributes.
static constexpr size_t STUN_HEADER_SIZE = 20;
// Size of attribute header
static constexpr size_t STUN_ATTR_HEADER_SIZE = 4;
// The magic cookie field MUST contain the fixed value 0x2112A442 in
// network byte order.
static constexpr uint32_t MAGIC_COOKIE = 0x2112A442;

std::optional<Message> Message::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    if (vv.size() < STUN_HEADER_SIZE) {
        stat.error.inc();
        stat.invalid_size.inc();
        return std::nullopt;
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
        return std::nullopt;
    }
    if (msg_length != vv.size() - STUN_HEADER_SIZE) {
        stat.error.inc();
        stat.message_length_error.inc();
        return std::nullopt;
    }

    const auto cls = Class::from_msg_type(msg_type);
    const IsRFC3489 is_rfc3489{cls.value() == Class::REQUEST && magic_cookie != MAGIC_COOKIE};
    if (!is_rfc3489 && magic_cookie != MAGIC_COOKIE) {
        stat.error.inc();
        stat.magic_cookie_error.inc();
        return std::nullopt;
    }

    const auto transaction_id = !is_rfc3489 ? vv.assured_subview(8, 12) : vv.assured_subview(4, 12);

    AttributeSet attrs;
    std::optional<util::ConstBinaryView::Interval> integrity;
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
            return std::nullopt;
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
        // RFC5389:
        // 15.4.  MESSAGE-INTEGRITY
        // With the exception of the FINGERPRINT attribute, which
        // appears after MESSAGE-INTEGRITY, agents MUST ignore all
        // other attributes that follow MESSAGE-INTEGRITY.
        if (!integrity.has_value() || type == attr_registry::FINGERPRINT) {
            auto maybe_attr = Attribute::parse(attr_view, attr_type, stat);
            if (!maybe_attr.has_value()) {
                // statisitics is increased by Attribute::parse.
                return std::nullopt;
            }
            const auto& attr = *maybe_attr;
            if (attr.as<MessageIntegityAttribute>() != nullptr) {
                // 15.4.  MESSAGE-INTEGRITY
                // The text used as input to HMAC is the STUN message,
                // including the header, up to and including the
                // attribute preceding the MESSAGE-INTEGRITY
                // attribute.
                integrity = util::ConstBinaryView::Interval{0, attr_offset};
            }
            attrs.emplace(std::move(*maybe_attr));
        }

        attr_offset += align_length + STUN_ATTR_HEADER_SIZE;
    }

    stat.success.inc();
    return Message {
        Header {
            cls,
            Method::from_msg_type(msg_type),
            TransactionId(transaction_id)
         },
         std::move(attrs),
         is_rfc3489
    };
}


}
