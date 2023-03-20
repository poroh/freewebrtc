//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Message functions
//

#include "stun/stun_message.hpp"

namespace freewebrtc::stun {

// RFC5389: All STUN messages MUST start with a 20-byte header followed by zero
// or more Attributes.
static constexpr size_t STUN_HEADER_SIZE = 20;

std::optional<Message> Message::parse(const util::ConstBinaryView& vv, ParseStat& stat) {
    if (vv.size() < STUN_HEADER_SIZE) {
        stat.error.inc();
        stat.invalid_size.inc();
        return std::nullopt;
    }
    std::vector<Attribute> attrs;
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
        attr_offset += align_length;

        auto maybe_attr = Attribute::parse(attr_view, type, stat);
        if (!maybe_attr.has_value()) {
            // statisitics is increased by Attribute::parse.
            return std::nullopt;
        }
        attrs.emplace_back(std::move(*maybe_attr));
    }

    stat.success.inc();
    return std::nullopt;
}


}
