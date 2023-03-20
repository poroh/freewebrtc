//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Attribute
//

#pragma once

#include <cstdint>
#include <optional>
#include <variant>

#include "util/util_binary_view.hpp"
#include "net/ip/ip_address.hpp"
#include "net/net_port.hpp"

namespace freewebrtc::stun {

struct ParseStat;

struct UnknownAttribute {
    UnknownAttribute(const util::ConstBinaryView&, uint16_t type);
    UnknownAttribute(UnknownAttribute&&) = default;

    uint16_t type;
    std::vector<uint8_t> data;
};

struct MappedAddressAttribute {
    net::ip::Address address;
    net::Port port;
    static std::optional<MappedAddressAttribute> parse(const util::ConstBinaryView&, ParseStat&);
};

class Attribute {
public:
    using Value = std::variant<MappedAddressAttribute, UnknownAttribute>;
    static std::optional<Attribute> parse(const util::ConstBinaryView&, uint16_t type, ParseStat&);
private:
    Attribute(Value&&);
    Value m_value;
};

}
