//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Hash for IP address to use it in unordered_map / unordered_set
//

#include "net/ip/ip_address_hash.hpp"
#include "util/util_variant_overloaded.hpp"
#include "util/util_hash_murmur.hpp"

namespace freewebrtc::net::ip {

std::size_t AddressHash::operator()(const net::ip::Address& addr) {
    return std::visit(
        util::overloaded {
            [](const AddressV4& v4) {
                std::size_t result = 0;
                auto view = v4.view();
                memcpy(&result, view.data(), std::min(view.size(), sizeof(std::size_t)));
                return result;
            },
            [](const AddressV6& v6) {
                static constexpr uint64_t murmur_seed = 0;
                return util::hash::murmur(v6.view(), murmur_seed);
            }},
        addr.value());
}

}
