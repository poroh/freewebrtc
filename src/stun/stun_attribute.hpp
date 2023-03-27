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
#include "stun/stun_attribute_type.hpp"
#include "crypto/crypto_hmac.hpp"
#include "crypto/crypto_hash.hpp"

namespace freewebrtc::stun {

struct ParseStat;

struct UnknownAttribute {
    UnknownAttribute(const util::ConstBinaryView&);
    UnknownAttribute(UnknownAttribute&&) = default;

    std::vector<uint8_t> data;
};

struct AddressAttribute {
    net::ip::Address address;
    net::Port port;
};

struct MappedAddressAttribute : public AddressAttribute {
    static std::optional<MappedAddressAttribute> parse(const util::ConstBinaryView&, ParseStat&);
};

struct XorMappedAddressAttribute : public AddressAttribute {
    static std::optional<XorMappedAddressAttribute> parse(const util::ConstBinaryView&, ParseStat&);
};

struct MessageIntegityAttribute {
    using Digest = crypto::hmac::Digest<crypto::SHA1Hash>;
    Digest digest;
    static std::optional<MessageIntegityAttribute> parse(const util::ConstBinaryView&, ParseStat&);
};

struct FingerprintAttribute {
    uint32_t crc32;
    static std::optional<FingerprintAttribute> parse(const util::ConstBinaryView&, ParseStat&);
};

class Attribute {
public:
    using Value =
        std::variant<
            XorMappedAddressAttribute,
            MappedAddressAttribute,
            MessageIntegityAttribute,
            FingerprintAttribute,
            UnknownAttribute
        >;

    AttributeType type() const noexcept;

    template<typename AttrType>
    const AttrType *as() const noexcept;

    static std::optional<Attribute> parse(const util::ConstBinaryView&, AttributeType type, ParseStat&);
private:
    Attribute(AttributeType, Value&&);
    AttributeType m_type;
    Value m_value;
};

//
// inlines
//
inline AttributeType Attribute::type() const noexcept {
    return m_type;
}

template<typename AttrType>
inline const AttrType *Attribute::as() const noexcept {
    return std::get_if<AttrType>(&m_value);
}

}
