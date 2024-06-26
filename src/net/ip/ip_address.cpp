//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//

#include <arpa/inet.h>
#include <string>

#include "net/ip/ip_address.hpp"
#include "util/util_variant_overloaded.hpp"

namespace freewebrtc::net::ip {

template<typename AddressType, typename InetStruct, int INET_FAMILY>
Result<AddressType> inet_pton_adapter(const std::string_view& v) {
    InetStruct ip_addr;
    // IEEE Std 1003.1-2017:
    // The inet_pton() function shall return 1 if the conversion
    // succeeds, with the address pointed to by dst in network byte
    // order.
    std::string copy(v.begin(), v.end());
    int rc = inet_pton(INET_FAMILY, copy.c_str(), &ip_addr);
    if (rc > 0) {
        return AddressType::from_view(util::ConstBinaryView(&ip_addr, sizeof(ip_addr)));
    }
    if (rc == 0) {
        return make_error_code(Error::ip_address_parse_error);
    }
    return std::error_code(errno, std::generic_category());
}

Result<AddressV4> from_string_v4(const std::string_view& v) {
    return inet_pton_adapter<AddressV4, struct in_addr, AF_INET>(v);
}

Result<AddressV6> from_string_v6(const std::string_view& v) {
    return inet_pton_adapter<AddressV6, struct in6_addr, AF_INET6>(v);
}

Result<Address> Address::from_string(const std::string_view& v) {
    return from_string_v4(v)
        .fmap(Address::move_from_v4)
        .bind_err([&](auto&&) {
            return from_string_v6(v)
                .fmap(Address::move_from_v6);
        });
}

Result<std::string> Address::to_string() const {
    const auto& p = std::visit(util::overloaded {
            [](const AddressV4& v) { return std::make_pair(v.view(), AF_INET); },
            [](const AddressV6& v) { return std::make_pair(v.view(), AF_INET6); },
        }, m_value);
    std::vector<char> buffer(INET6_ADDRSTRLEN);
    const char *result = nullptr;
    while ((result = inet_ntop(p.second, p.first.data(), buffer.data(), buffer.size())) == nullptr) {
        if (errno != ENOSPC) {
            return std::error_code(errno, std::generic_category());
        }
        buffer.resize(buffer.size() * 2);
    }
    return std::string(result);
}

util::ConstBinaryView Address::view() const noexcept {
    return
        std::visit(
            util::overloaded {
                [](const AddressV4& v) { return v.view(); },
                [](const AddressV6& v) { return v.view(); }
                },
            m_value);
}


}

