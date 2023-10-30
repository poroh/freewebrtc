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

#include "util/util_endian.hpp"

namespace freewebrtc::util {

class ConstBinaryView : private std::basic_string_view<const uint8_t> {
    using Base = std::basic_string_view<value_type>;
public:
    using ByteT = uint8_t;
    using ByteVec = std::vector<ByteT>;
    struct Interval {
        size_t offset;
        size_t count;
    };
    ConstBinaryView(const void *, size_t count);
    explicit ConstBinaryView(const ByteVec&);

    template<size_t SIZE>
    explicit ConstBinaryView(const std::array<ByteT, SIZE>&);

    bool contains(const Interval& i) const;
    using Base::data;
    using Base::size;
    using Base::begin;
    using Base::end;

    uint8_t assured_read_u8(size_t offset) const;
    uint16_t assured_read_u16be(size_t offset) const;
    uint32_t assured_read_u32be(size_t offset) const;
    uint64_t assured_read_u64be(size_t offset) const;
    ConstBinaryView assured_subview(size_t offset, size_t size) const;

    std::optional<uint8_t> read_u8(size_t offset) const;
    std::optional<uint16_t> read_u16be(size_t offset) const;
    std::optional<uint32_t> read_u32be(size_t offset) const;
    std::optional<uint64_t> read_u64be(size_t offset) const;
    std::optional<ConstBinaryView> subview(size_t offset) const;
    std::optional<ConstBinaryView> subview(size_t offset, size_t size) const;
    std::optional<ConstBinaryView> subview(const Interval&) const;

    static ByteVec concat(const std::vector<ConstBinaryView>&);

    bool operator==(const ConstBinaryView&) const noexcept;
};

using ByteVec = ConstBinaryView::ByteVec;

//
// inlines
//
inline ConstBinaryView::ConstBinaryView(const std::vector<uint8_t>& v)
    : ConstBinaryView(v.data(), v.size())
{}

template<size_t SIZE>
inline ConstBinaryView::ConstBinaryView(const std::array<uint8_t, SIZE>& v)
    : ConstBinaryView(v.data(), v.size())
{}

inline ConstBinaryView::ConstBinaryView(const void *buf, size_t count)
    : Base((ByteT *)buf, count)
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

inline uint64_t ConstBinaryView::assured_read_u64be(size_t offset) const {
    uint64_t value;
    memcpy(&value, &data()[offset], sizeof(value));
    return network_to_host_u64(value);
}

inline ConstBinaryView ConstBinaryView::assured_subview(size_t offset, size_t count) const {
    return ConstBinaryView(data() + offset, count);
}

inline std::optional<uint8_t> ConstBinaryView::read_u8(size_t offset) const {
    if (offset + sizeof(uint8_t) > size()) {
        return std::nullopt;
    }
    return assured_read_u8(offset);
}

inline std::optional<uint16_t> ConstBinaryView::read_u16be(size_t offset) const {
    if (offset + sizeof(uint16_t) > size() ) {
        return std::nullopt;
    }
    return assured_read_u16be(offset);
}

inline std::optional<uint32_t> ConstBinaryView::read_u32be(size_t offset) const {
    if (offset + sizeof(uint32_t) > size() ) {
        return std::nullopt;
    }
    return assured_read_u32be(offset);
}

inline std::optional<uint64_t> ConstBinaryView::read_u64be(size_t offset) const {
    if (offset + sizeof(uint64_t) > size() ) {
        return std::nullopt;
    }
    return assured_read_u64be(offset);
}

inline bool ConstBinaryView::contains(const Interval& i) const {
    return i.offset + i.count <= size();
}

inline std::optional<ConstBinaryView> ConstBinaryView::subview(size_t offset) const {
    if (offset > size()) {
        return std::nullopt;
    }
    return subview(offset, size() - offset);
}

inline std::optional<ConstBinaryView> ConstBinaryView::subview(size_t offset, size_t count) const {
    if (offset + count > size()) {
        return std::nullopt;
    }
    return assured_subview(offset, count);
}

inline std::optional<ConstBinaryView> ConstBinaryView::subview(const Interval& i) const {
    return subview(i.offset, i.count);
}

inline bool ConstBinaryView::operator==(const ConstBinaryView& other) const noexcept {
    return static_cast<const Base&>(*this) == static_cast<const Base&>(other);
}

}
