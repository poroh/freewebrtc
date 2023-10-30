//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Network endpoint (IP address + port)
//

#pragma once

#include <variant>
#include "net/ip/ip_address.hpp"
#include "net/net_port.hpp"
#include "util/util_variant_overloaded.hpp"

namespace freewebrtc::net {

template<typename Tag>
struct GenericEndpoint {
    ip::Address address;
    net::Port port;
};

struct UdpEndpointTag{};
struct TcpEndpointTag{};

using UdpEndpoint = GenericEndpoint<UdpEndpointTag>;
using TcpEndpoint = GenericEndpoint<TcpEndpointTag>;

class Endpoint {
public:
    using Value = std::variant<UdpEndpoint, TcpEndpoint>;
    Endpoint(const UdpEndpoint&);
    Endpoint(const TcpEndpoint&);
    Endpoint(UdpEndpoint&&);
    Endpoint(TcpEndpoint&&);

    bool is_udp() const noexcept;
    ip::Address address() const noexcept;
    net::Port port() const noexcept;

private:
    Value m_value;
};

//
// implementation
//
inline Endpoint::Endpoint(const UdpEndpoint& v)
    : m_value(v)
{}

inline Endpoint::Endpoint(const TcpEndpoint& v)
    : m_value(v)
{}

inline Endpoint::Endpoint(UdpEndpoint&& v)
    : m_value(std::move(v))
{}

inline Endpoint::Endpoint(TcpEndpoint&& v)
    : m_value(std::move(v))
{}

inline bool Endpoint::is_udp() const noexcept {
    return std::holds_alternative<UdpEndpoint>(m_value);
}

inline ip::Address Endpoint::address() const noexcept {
    return std::visit(
        util::overloaded {
            [](const net::UdpEndpoint& ep) { return ep.address; },
            [](const net::TcpEndpoint& ep) { return ep.address; }
        },
        m_value);
}

inline net::Port Endpoint::port() const noexcept {
    return std::visit(
        util::overloaded {
            [](const net::UdpEndpoint& ep) { return ep.port; },
            [](const net::TcpEndpoint& ep) { return ep.port; }
        },
        m_value);
}

}


