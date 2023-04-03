//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// IP address template
//

#pragma once

#include <array>
#include <optional>
#include "util/util_binary_view.hpp"

namespace freewebrtc::net::ip::details {

template<size_t SIZE>
class Address {
public:
    using Value = std::array<uint8_t, SIZE>;

    static constexpr size_t size();
    static std::optional<Address> from_view(const util::ConstBinaryView&);

    Address(Value&&);
    util::ConstBinaryView view() const noexcept;

    bool operator==(const Address&) const noexcept = default;
private:
    std::array<uint8_t, SIZE> m_value;
};

//
// implementation
//
template<size_t SIZE>
inline Address<SIZE>::Address(Value&& v)
    : m_value(std::move(v))
{}

template<size_t SIZE>
constexpr size_t Address<SIZE>::size() {
    return SIZE;
}

template<size_t SIZE>
inline util::ConstBinaryView Address<SIZE>::view() const noexcept {
    return util::ConstBinaryView(m_value);
}

template<size_t SIZE>
inline std::optional<Address<SIZE>> Address<SIZE>::from_view(const util::ConstBinaryView& vv) {
    if (vv.size() != std::tuple_size<Value>::value) {
        return std::nullopt;
    }
    Value v;
    std::copy(vv.begin(), vv.end(), v.begin());
    return Address(std::move(v));
}

}
