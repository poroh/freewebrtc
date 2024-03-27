//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Address
//

#pragma once

#include <string_view>
#include <variant>
#include <functional>

#include "net/ip/ip_address.hpp"
#include "net/net_fqdn.hpp"

namespace freewebrtc::ice::candidate {

class Address {
public:
    using FqdnCRef = std::reference_wrapper<const net::Fqdn>;
    using IpAddressCRef = std::reference_wrapper<const net::ip::Address>;
    using MaybeFqdnCRef = std::optional<FqdnCRef>;
    using MaybeIpAddressCRef = std::optional<IpAddressCRef>;

    Address(const Address&) = default;
    Address(Address&&) = default;
    Address& operator=(const Address&) = default;
    Address& operator=(Address&&) = default;

    static Result<Address> from_string(std::string_view v) noexcept;

    MaybeFqdnCRef as_fqdn() const noexcept;
    MaybeIpAddressCRef as_ip_address() const noexcept;

    Result<std::string> to_string() const;

private:
    using Value = std::variant<net::ip::Address, net::Fqdn>;
    Address(net::ip::Address&&);
    Address(net::Fqdn&&);
    Value m_value;
};

//
// implementation
//
inline Address::Address(net::ip::Address&& v)
    : m_value(std::move(v))
{}

inline Address::Address(net::Fqdn&& v)
    : m_value(std::move(v))
{}

inline Address::MaybeFqdnCRef Address::as_fqdn() const noexcept {
    if (std::holds_alternative<net::Fqdn>(m_value)) {
        return std::cref(std::get<net::Fqdn>(m_value));
    }
    return std::nullopt;
}

inline Address::MaybeIpAddressCRef Address::as_ip_address() const noexcept{
    if (std::holds_alternative<net::ip::Address>(m_value)) {
        return std::cref(std::get<net::ip::Address>(m_value));
    }
    return std::nullopt;
}

inline Result<std::string> Address::to_string() const {
    return std::visit([](auto&& v) {
        if constexpr (std::is_same_v<const net::ip::Address&, decltype(v)>) {
            return v.to_string();
        } else if constexpr (std::is_same_v<const net::Fqdn&, decltype(v)>) {
            return Result<std::string>{std::move(v.to_string())};
        }
    }, m_value);
}


}
