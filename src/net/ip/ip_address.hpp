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
#include <optional>
#include <string_view>
#include "net/ip/ip_address_v4.hpp"
#include "net/ip/ip_address_v6.hpp"
#include "util/util_return_value.hpp"

namespace freewebrtc::net::ip {

class Address {
public:
    using Value = std::variant<AddressV4, AddressV6>;

    Address(const AddressV4&);
    Address(const AddressV6&);
    Address(AddressV4&&);
    Address(AddressV6&&);

    static ReturnValue<Address> from_string(const std::string_view&);

    const Value& value() const noexcept;

    ReturnValue<std::string> to_string() const;
    bool operator==(const Address&) const noexcept = default;
private:
    Value m_value;
};


//
// implementation
//

inline Address::Address(const AddressV4& v)
    : m_value(v)
{}

inline Address::Address(const AddressV6& v)
    : m_value(v)
{}

inline Address::Address(AddressV4&& v)
    : m_value(std::move(v))
{}

inline Address::Address(AddressV6&& v)
    : m_value(std::move(v))
{}

inline const Address::Value& Address::value() const noexcept {
    return m_value;
}

}
