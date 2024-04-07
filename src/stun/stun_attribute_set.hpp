//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Attribute container
//

#pragma once

#include <unordered_map>

#include "util/util_maybe.hpp"
#include "stun/stun_attribute.hpp"
#include "stun/stun_integrity.hpp"
#include "stun/details/stun_attr_registry.hpp"

namespace freewebrtc::stun {

struct Header;

class AttributeSet {
public:
    void emplace(Attribute&&);
    void emplace(UnknownAttribute&&);
    template<typename Attr>
    using MaybeAttr = Maybe<std::reference_wrapper<const Attr>>;
    MaybeAttr<MessageIntegityAttribute::Digest> integrity() const noexcept;
    MaybeAttr<precis::OpaqueString> username() const noexcept;
    MaybeAttr<std::string> software() const noexcept;
    MaybeAttr<XorMappedAddressAttribute> xor_mapped() const noexcept;
    MaybeAttr<MappedAddressAttribute> mapped() const noexcept;
    MaybeAttr<uint32_t> priority() const noexcept;
    MaybeAttr<uint64_t> ice_controlling() const noexcept;
    MaybeAttr<uint64_t> ice_controlled() const noexcept;
    bool has_use_candidate() const noexcept;
    MaybeAttr<ErrorCodeAttribute> error_code() const noexcept;
    MaybeAttr<UnknownAttributesAttribute> unknown_attributes() const noexcept;
    MaybeAttr<AlternateServerAttribute> alternate_server() const noexcept;
    // Return list of uknown comprehension-required attributes
    std::vector<AttributeType> unknown_comprehension_required() const noexcept;
    bool has_fingerprint() const noexcept;

    using AttrVec = std::vector<Attribute::Value>;
    using UnknownAttrVec = std::vector<UnknownAttribute>;
    static AttributeSet create(AttrVec&&, UnknownAttrVec&& = {});

    Result<util::ByteVec> build(const Header&, const MaybeIntegrity&) const;
private:
    std::unordered_map<AttributeType, Attribute> m_map;
    std::vector<UnknownAttribute> m_unknown;
};

//
// inlines
//
inline void AttributeSet::emplace(Attribute&& attr)  {
    if (m_map.find(attr.type()) != m_map.end()) {
        return;
    }
    m_map.emplace(attr.type(), std::move(attr));
}

inline void AttributeSet::emplace(UnknownAttribute&& attr) {
    m_unknown.emplace_back(std::move(attr));
}

}
