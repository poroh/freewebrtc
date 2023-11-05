//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Address
//

#pragma once

#include <cstdint>
#include <optional>
#include "net/ip/ip_address.hpp"
#include "util/util_binary_view.hpp"

namespace freewebrtc::stun {

class TransactionId;

class Family {
public:
    enum Type {
        IPv4,
        IPv6
    };
    static Family ipv4() noexcept;
    static Family ipv6() noexcept;
    static std::optional<Family> from_uint8(std::optional<uint8_t>);

    Type type() const noexcept;
    uint8_t to_uint8() const noexcept;
private:
    explicit Family(Type);
    Type m_type;
};

class XoredAddress {
public:
    static std::optional<XoredAddress> from_view(Family, const util::ConstBinaryView&);
    static XoredAddress from_address(const net::ip::Address&, const TransactionId&);
    // Get Address from XOR format.
    // Normative RFC8489 (14.2.  XOR-MAPPED-ADDRESS)
    // If the IP address family is IPv4, X-Address is computed by
    // XOR'ing the mapped IP address with the magic cookie.  If the IP
    // address family is IPv6, X-Address is computed by XOR'ing the
    // mapped IP address with the concatenation of the magic cookie
    // and the 96-bit transaction ID.
    net::ip::Address to_address(const TransactionId&) const noexcept;
    Family family() const noexcept;
    util::ConstBinaryView view() const noexcept;
    bool operator==(const XoredAddress&) const noexcept = default;
private:
    using V4Holder = std::array<uint8_t, net::ip::AddressV4::size()>;
    using V6Holder = std::array<uint8_t, net::ip::AddressV6::size()>;
    using Value = std::variant<V4Holder, V6Holder>;
    XoredAddress(Value&&);
    Value m_value;
};


//
// implementation
//
inline Family::Family(Type type)
    : m_type(type)
{}

inline Family::Type Family::type() const noexcept {
    return m_type;
}

inline Family Family::ipv4() noexcept {
    return Family(IPv4);
}

inline Family Family::ipv6() noexcept {
    return Family(IPv6);
}

}
