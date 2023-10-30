//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Address
//

#include "stun/stun_address.hpp"
#include "stun/stun_transaction_id.hpp"
#include "stun/details/stun_attr_registry.hpp"
#include "stun/details/stun_constants.hpp"
#include "util/util_variant_overloaded.hpp"

namespace freewebrtc::stun {

std::optional<Family> Family::from_uint8(std::optional<uint8_t> maybe_v) {
    if (!maybe_v.has_value()) {
        return std::nullopt;
    }
    switch (*maybe_v) {
        case attr_registry::FAMILY_IPV4:
            return Family(Family::IPv4);
        case attr_registry::FAMILY_IPV6:
            return Family(Family::IPv6);
    }
    return std::nullopt;
}

uint8_t Family::to_uint8() const noexcept {
    switch (m_type) {
    case IPv4:
        return attr_registry::FAMILY_IPV4;
    case IPv6:
        return attr_registry::FAMILY_IPV6;
    }
}

std::optional<XoredAddress> XoredAddress::from_view(Family f, const util::ConstBinaryView& vv) {
    switch (f.type()) {
        case Family::IPv4: {
            if (vv.size() != std::tuple_size<V4Holder>::value) {
                return std::nullopt;
            }
            V4Holder holder;
            std::copy(vv.begin(), vv.end(), holder.begin());
            return XoredAddress(std::move(holder));
        }
        case Family::IPv6: {
            if (vv.size() != std::tuple_size<V6Holder>::value) {
                return std::nullopt;
            }
            V6Holder holder;
            std::copy(vv.begin(), vv.end(), holder.begin());
            return XoredAddress(std::move(holder));
        }
    }
}

XoredAddress::XoredAddress(Value&& v)
    : m_value(std::move(v))
{}

net::ip::Address XoredAddress::to_address(const TransactionId& tid) const noexcept {
    return std::visit(
        util::overloaded {
            [](const V4Holder& v) {
                V4Holder vv = v;
                const uint32_t magic = util::host_to_network_u32(details::MAGIC_COOKIE);
                auto& vvu32 = reinterpret_cast<uint32_t&>(*vv.begin());
                vvu32 ^= magic;
                return net::ip::Address(net::ip::AddressV4(std::move(vv)));
            },
            [&](const V6Holder& v) {
                // If the IP address family is IPv6, X-Address is
                // computed by XOR'ing the mapped IP address with the
                // concatenation of the magic cookie and the 96-bit
                // transaction ID.
                V6Holder vv = v;
                const uint32_t magic = util::host_to_network_u32(details::MAGIC_COOKIE);
                auto& vvu32 = reinterpret_cast<uint32_t&>(*vv.begin());
                vvu32 ^= magic;
                auto tidv = tid.view();
                for (size_t i = 0; i < tidv.size() && i < vv.size() - 4; ++i) {
                    vv[i + 4] ^= tidv.assured_read_u8(i);
                }
                return net::ip::Address(net::ip::AddressV6(std::move(vv)));
            }
        }, m_value);
}

Family XoredAddress::family() const noexcept {
    return std::visit(
        util::overloaded {
            [](const V4Holder&) {
                return Family::ipv4();
            },
            [&](const V6Holder&) {
                return Family::ipv6();
            }
        }, m_value);
}

util::ConstBinaryView XoredAddress::view() const noexcept {
    return std::visit(
        util::overloaded {
            [](const V4Holder& h4) {
                return util::ConstBinaryView(h4);
            },
            [&](const V6Holder& h6) {
                return util::ConstBinaryView(h6);
            }
        }, m_value);
}



}
