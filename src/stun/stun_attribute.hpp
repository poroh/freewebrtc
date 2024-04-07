//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Attribute
//

#pragma once

#include <cstdint>
#include <variant>

#include "util/util_binary_view.hpp"
#include "util/util_result.hpp"
#include "net/ip/ip_address.hpp"
#include "net/net_port.hpp"
#include "stun/stun_attribute_type.hpp"
#include "stun/stun_address.hpp"
#include "crypto/crypto_hmac.hpp"
#include "crypto/crypto_hash.hpp"
#include "precis/precis_opaque_string.hpp"

namespace freewebrtc::stun {

struct ParseStat;

struct UnknownAttribute {
    UnknownAttribute(AttributeType type, const util::ConstBinaryView&);
    UnknownAttribute(UnknownAttribute&&) = default;
    UnknownAttribute(const UnknownAttribute&) = default;
    UnknownAttribute& operator=(const UnknownAttribute&) = default;
    UnknownAttribute& operator=(UnknownAttribute&&) = default;

    AttributeType type;
    std::vector<uint8_t> data;
};

struct MappedAddressAttribute {
    net::ip::Address addr;
    net::Port port;
    static Result<MappedAddressAttribute> parse(const util::ConstBinaryView&, ParseStat&);
    util::ByteVec build() const;
};

struct XorMappedAddressAttribute {
    XoredAddress addr;
    net::Port port;
    bool operator==(const XorMappedAddressAttribute&) const noexcept = default;
    static Result<XorMappedAddressAttribute> parse(const util::ConstBinaryView&, ParseStat&);
    util::ByteVec build() const;
};

struct UsernameAttribute {
    precis::OpaqueString name;
    static Result<UsernameAttribute> parse(const util::ConstBinaryView&, ParseStat&);
};

struct SoftwareAttribute {
    std::string name;
    static Result<SoftwareAttribute> parse(const util::ConstBinaryView&, ParseStat&);
};

struct MessageIntegityAttribute {
    using Digest = crypto::hmac::Digest<crypto::SHA1Hash>;
    Digest digest;
    static Result<MessageIntegityAttribute> parse(const util::ConstBinaryView&, ParseStat&);
};

struct FingerprintAttribute {
    uint32_t crc32;
    static Result<FingerprintAttribute> parse(const util::ConstBinaryView&, ParseStat&);
};

struct PriorityAttribute {
    uint32_t priority;
    static Result<PriorityAttribute> parse(const util::ConstBinaryView&, ParseStat&);
};

struct UseCandidateAttribute {
    static Result<UseCandidateAttribute> parse(const util::ConstBinaryView&, ParseStat&);
};

struct IceControllingAttribute {
    uint64_t tiebreaker;
    static Result<IceControllingAttribute> parse(const util::ConstBinaryView&, ParseStat&);
};

struct IceControlledAttribute {
    uint64_t tiebreaker;
    static Result<IceControlledAttribute> parse(const util::ConstBinaryView&, ParseStat&);
};

struct UnknownAttributesAttribute {
    std::vector<AttributeType> types;
    static Result<UnknownAttributesAttribute> parse(const util::ConstBinaryView&, ParseStat&);
    util::ByteVec build() const;
};

struct ErrorCodeAttribute {
    // 15.6.  ERROR-CODE
    enum Code {
        TryAlternate     = 300,
        BadRequest       = 400,
        Unauthorized     = 401,
        UnknownAttribute = 420,
        StaleNonce       = 438,
        ServerError      = 500
    };
    unsigned code;
    Maybe<std::string> reason_phrase;

    bool operator==(const ErrorCodeAttribute&) const noexcept;
    static Result<ErrorCodeAttribute> parse(const util::ConstBinaryView&, ParseStat&);
    util::ByteVec build() const;
};

struct AlternateServerAttribute {
    net::ip::Address addr;
    net::Port port;
    static Result<AlternateServerAttribute> parse(const util::ConstBinaryView&, ParseStat&);
    util::ByteVec build() const;
};

class Attribute {
public:
    using Value =
        std::variant<
            XorMappedAddressAttribute,
            MappedAddressAttribute,
            UsernameAttribute,
            SoftwareAttribute,
            MessageIntegityAttribute,
            FingerprintAttribute,
            PriorityAttribute,
            IceControllingAttribute,
            IceControlledAttribute,
            UseCandidateAttribute,
            UnknownAttributesAttribute,
            ErrorCodeAttribute,
            AlternateServerAttribute
        >;

    AttributeType type() const noexcept;
    const Value& value() const noexcept;

    template<typename AttrType>
    const AttrType *as() const noexcept;

    using ParseResult = std::variant<Attribute, UnknownAttribute>;
    static Result<ParseResult> parse(const util::ConstBinaryView&, AttributeType type, ParseStat&);
    static Attribute create(Value&&);

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

inline const Attribute::Value& Attribute::value() const noexcept {
    return m_value;
}

}
