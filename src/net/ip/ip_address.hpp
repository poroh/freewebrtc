//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// IP address (unfortunatelly no stanard IP address in C++ :(
// boost::asio::ip::address could be used instead of it but
// I avoid boost dependency in favor of standard C++ and Posix
// libraries only.
//

#pragma once

#include <variant>
#include <string_view>
#include "net/ip/ip_address_v4.hpp"
#include "net/ip/ip_address_v6.hpp"
#include "util/util_result.hpp"

namespace freewebrtc::net::ip {

class Address {
public:
    using Value = std::variant<AddressV4, AddressV6>;

    Address(const AddressV4&) noexcept;
    Address(const AddressV6&) noexcept;
    Address(AddressV4&&) noexcept;
    Address(AddressV6&&) noexcept;

    static Address move_from_v4(AddressV4&&) noexcept;
    static Address move_from_v6(AddressV6&&) noexcept;
    static Address copy_from_v4(const AddressV4&) noexcept;
    static Address copy_from_v6(const AddressV6&) noexcept;

    static Result<Address> from_string(const std::string_view&);

    const Value& value() const noexcept;

    Result<std::string> to_string() const;
    bool operator==(const Address&) const noexcept = default;

    util::ConstBinaryView view() const noexcept;
private:
    Value m_value;
};

//
// implementation
//

inline Address::Address(const AddressV4& v) noexcept
    : m_value(v)
{}

inline Address::Address(const AddressV6& v) noexcept
    : m_value(v)
{}

inline Address::Address(AddressV4&& v) noexcept
    : m_value(std::move(v))
{}

inline Address::Address(AddressV6&& v) noexcept
    : m_value(std::move(v))
{}

inline Address Address::move_from_v4(AddressV4&& addr) noexcept {
    return Address{std::move(addr)};
}

inline Address Address::move_from_v6(AddressV6&& addr) noexcept {
    return Address{std::move(addr)};
}

inline Address Address::copy_from_v4(const AddressV4& addr) noexcept {
    return Address{std::move(addr)};
}

inline Address Address::copy_from_v6(const AddressV6& addr) noexcept {
    return Address{std::move(addr)};
}

inline const Address::Value& Address::value() const noexcept {
    return m_value;
}

}
