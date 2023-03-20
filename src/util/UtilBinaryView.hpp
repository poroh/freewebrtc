//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Binary packet view (does not contain own data, just works over existing buffer)
//

#pragma once

#include <string_view>
#include <cstdint>
#include <optional>
#include <vector>

#include "util/UtilEndian.hpp"


namespace freewebrtc::util {

class ConstBinaryView : private std::basic_string_view<const uint8_t> {
    using ByteT = uint8_t;
    using Base = std::basic_string_view<const ByteT>;
public:
    struct Interval {
        size_t offset;
        size_t count;
    };
    ConstBinaryView(const ByteT *, size_t count);
    explicit ConstBinaryView(const std::vector<uint8_t>&);

    bool contains(const Interval& i) const;
    using Base::data;
    using Base::size;

    uint8_t assured_read_u8(size_t offset) const;
    uint16_t assured_read_u16be(size_t offset) const;
    uint32_t assured_read_u32be(size_t offset) const;

    std::optional<uint8_t> read_u8(size_t offset) const;
    std::optional<uint16_t> read_u16be(size_t offset) const;
    std::optional<uint32_t> read_u32be(size_t offset) const;
    std::optional<ConstBinaryView> subview(size_t offset, size_t size) const;

};


//
// inlines
//
inline ConstBinaryView::ConstBinaryView(const std::vector<uint8_t>& v)
    : ConstBinaryView(v.data(), v.size())
{}


inline ConstBinaryView::ConstBinaryView(const ByteT *buf, size_t count)
    : Base(buf, count)
{}


inline uint8_t ConstBinaryView::assured_read_u8(size_t offset) const {
    return data()[offset];
}

inline uint16_t ConstBinaryView::assured_read_u16be(size_t offset) const {
    uint16_t value;
    memcpy(&value, &data()[offset], sizeof(value));
    return network_to_host_u16(value);
}

inline uint32_t ConstBinaryView::assured_read_u32be(size_t offset) const {
    uint32_t value;
    memcpy(&value, &data()[offset], sizeof(value));
    return network_to_host_u32(value);
}

inline std::optional<uint8_t> ConstBinaryView::read_u8(size_t offset) const {
    if (offset > size() - sizeof(uint8_t)) {
        return std::nullopt;
    }
    return assured_read_u8(offset);
}

inline std::optional<uint16_t> ConstBinaryView::read_u16be(size_t offset) const {
    if (offset > size() - sizeof(uint16_t)) {
        return std::nullopt;
    }
    return assured_read_u16be(offset);
}

inline std::optional<uint32_t> ConstBinaryView::read_u32be(size_t offset) const {
    if (offset > size() - sizeof(uint32_t)) {
        return std::nullopt;
    }
    return assured_read_u32be(offset);
}

inline bool ConstBinaryView::contains(const Interval& i) const {
    return i.offset + i.count <= size();
}

inline std::optional<ConstBinaryView> ConstBinaryView::subview(size_t offset, size_t count) const {
    if (count > size() || offset > size() - count) {
        return std::nullopt;
    }
    return ConstBinaryView(data() + offset, count);
}


}
