//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// IP address template
//

#pragma once

#include <array>
#include "net/net_error.hpp"
#include "util/util_binary_view.hpp"
#include "util/util_result.hpp"

namespace freewebrtc::net::ip::details {

template<size_t SIZE>
class Address {
public:
    using Value = std::array<uint8_t, SIZE>;

    static constexpr size_t size() noexcept;
    static Result<Address> from_view(const util::ConstBinaryView&) noexcept;

    Address(Value&&) noexcept;
    util::ConstBinaryView view() const noexcept;

    bool operator==(const Address&) const noexcept = default;
private:
    std::array<uint8_t, SIZE> m_value;
};

//
// implementation
//
template<size_t SIZE>
inline Address<SIZE>::Address(Value&& v) noexcept
    : m_value(std::move(v))
{}

template<size_t SIZE>
constexpr size_t Address<SIZE>::size() noexcept {
    return SIZE;
}

template<size_t SIZE>
inline util::ConstBinaryView Address<SIZE>::view() const noexcept {
    return util::ConstBinaryView(m_value);
}

template<size_t SIZE>
inline Result<Address<SIZE>> Address<SIZE>::from_view(const util::ConstBinaryView& vv) noexcept {
    if (vv.size() != std::tuple_size<Value>::value) {
        return make_error_code(Error::invalid_address_size);
    }
    Value v;
    std::copy(vv.begin(), vv.end(), v.begin());
    return Address(std::move(v));
}

}
