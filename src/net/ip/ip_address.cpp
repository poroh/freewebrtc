//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//

#include <arpa/inet.h>
#include <string>

#include "net/ip/ip_address.hpp"

namespace freewebrtc::net::ip {

template<typename AddressType, typename InetStruct, int INET_FAMILY>
std::optional<AddressType> inet_pton_adapter(const std::string_view& v) {
    InetStruct ip_addr;
    // IEEE Std 1003.1-2017:
    // The inet_pton() function shall return 1 if the conversion
    // succeeds, with the address pointed to by dst in network byte
    // order.
    std::string copy(v.begin(), v.end());
    if (inet_pton(INET_FAMILY, copy.c_str(), &ip_addr) != 1) {
        return std::nullopt;
    }
    return AddressType::from_view(util::ConstBinaryView(&ip_addr, sizeof(ip_addr)));
}

std::optional<AddressV4> from_string_v4(const std::string_view& v) {
    return inet_pton_adapter<AddressV4, struct in_addr, AF_INET>(v);
}

std::optional<AddressV6> from_string_v6(const std::string_view& v) {
    return inet_pton_adapter<AddressV6, struct in6_addr, AF_INET6>(v);
}

std::optional<Address> Address::from_string(const std::string_view& v) {
    if (auto maybe_addr = from_string_v4(v); maybe_addr.has_value()) {
        return std::move(maybe_addr);
    }
    return from_string_v6(v);
}

}

