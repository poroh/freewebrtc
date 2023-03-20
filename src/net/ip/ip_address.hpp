
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
#include "net/ip/ip_address_v4.hpp"
#include "net/ip/ip_address_v6.hpp"

namespace freewebrtc::net::ip {

class Address {
public:
    using Value = std::variant<AddressV4, AddressV6>;

    Address(const AddressV4&);
    Address(const AddressV6&);
    Address(AddressV4&&);
    Address(AddressV6&&);

private:
    Value m_value;
};

}
