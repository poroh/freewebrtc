//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Attribute container
//

#include "stun/stun_attribute_set.hpp"
#include "stun/stun_header.hpp"
#include "stun/details/stun_fingerprint.hpp"
#include "util/util_variant_overloaded.hpp"
#include <iostream>

namespace freewebrtc::stun {

AttributeSet::MaybeAttr<MessageIntegityAttribute::Digest> AttributeSet::integrity() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::MESSAGE_INTEGRITY)); it != m_map.end()) {
        return it->second.as<MessageIntegityAttribute>()->digest;
    }
    return std::nullopt;
}

AttributeSet::MaybeAttr<precis::OpaqueString> AttributeSet::username() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::USERNAME)); it != m_map.end()) {
        return it->second.as<UsernameAttribute>()->name;
    }
    return std::nullopt;
}

AttributeSet::MaybeAttr<std::string> AttributeSet::software() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::SOFTWARE)); it != m_map.end()) {
        return it->second.as<SoftwareAttribute>()->name;
    }
    return std::nullopt;
}

AttributeSet::MaybeAttr<XorMappedAddressAttribute> AttributeSet::xor_mapped() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::XOR_MAPPED_ADDRESS)); it != m_map.end()) {
        return *it->second.as<XorMappedAddressAttribute>();
    }
    return std::nullopt;
}

AttributeSet::MaybeAttr<MappedAddressAttribute> AttributeSet::mapped() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::MAPPED_ADDRESS)); it != m_map.end()) {
        return *it->second.as<MappedAddressAttribute>();
    }
    return std::nullopt;
}

AttributeSet::MaybeAttr<uint32_t> AttributeSet::priority() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::PRIORITY)); it != m_map.end()) {
        return it->second.as<PriorityAttribute>()->priority;
    }
    return std::nullopt;
}

AttributeSet::MaybeAttr<uint64_t> AttributeSet::ice_controlling() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::ICE_CONTROLLING)); it != m_map.end()) {
        return it->second.as<IceControllingAttribute>()->tiebreaker;
    }
    return std::nullopt;
}

AttributeSet::MaybeAttr<uint64_t> AttributeSet::ice_controlled() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::ICE_CONTROLLED)); it != m_map.end()) {
        return it->second.as<IceControlledAttribute>()->tiebreaker;
    }
    return std::nullopt;
}

AttributeSet::MaybeAttr<ErrorCodeAttribute> AttributeSet::error_code() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::ERROR_CODE)); it != m_map.end()) {
        return *it->second.as<ErrorCodeAttribute>();
    }
    return std::nullopt;
}

AttributeSet::MaybeAttr<UnknownAttributesAttribute> AttributeSet::unknown_attributes() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::UNKNOWN_ATTRIBUTES)); it != m_map.end()) {
        return *it->second.as<UnknownAttributesAttribute>();
    }
    return std::nullopt;
}

AttributeSet::MaybeAttr<AlternateServerAttribute> AttributeSet::alternate_server() const noexcept {
    if (auto it = m_map.find(AttributeType::from_uint16(attr_registry::ALTERNATE_SERVER)); it != m_map.end()) {
        return *it->second.as<AlternateServerAttribute>();
    }
    return std::nullopt;
}

bool AttributeSet::has_use_candidate() const noexcept {
    return m_map.find(AttributeType::from_uint16(attr_registry::USE_CANDIDATE)) != m_map.end();
}

std::vector<AttributeType> AttributeSet::unknown_comprehension_required() const noexcept {
    std::vector<AttributeType> result;
    for (const auto& p: m_unknown) {
        if (p.type.is_comprehension_required()) {
            result.emplace_back(p.type);
        }
    }
    return result;
}

bool AttributeSet::has_fingerprint() const noexcept {
    return m_map.find(AttributeType::from_uint16(attr_registry::FINGERPRINT)) != m_map.end();
}

AttributeSet AttributeSet::create(std::vector<Attribute::Value>&& ka, std::vector<UnknownAttribute>&& ua) {
    AttributeSet newset;
    for (auto&& a: ka) {
        newset.emplace(Attribute::create(std::move(a)));
    }
    for (auto&& a: ua) {
        newset.emplace(std::move(a));
    }
    return newset;
}

Result<util::ByteVec> AttributeSet::build(const Header& header, const MaybeIntegrity& maybe_integrity) const {
    std::vector<util::ConstBinaryView> result;
    const size_t num_attrs = m_map.size() + m_unknown.size() + (maybe_integrity.has_value() ? 1 : 0);
    result.reserve(num_attrs * 3 + 1); // We can have up to three views per attribute
    int dummy_hdr;
    result.emplace_back(&dummy_hdr, 0);

    std::vector<uint32_t> attr_headers(num_attrs);
    auto attr_headers_it = attr_headers.begin();
    const uint32_t padding = 0;
    uint32_t total_size = 0;

    auto add_attr = [&](auto type, const auto& view) {
        *attr_headers_it = util::host_to_network_u32((type << 16) | view.size());
        result.emplace_back(util::ConstBinaryView(&*attr_headers_it, sizeof(uint32_t)));
        total_size += sizeof(uint32_t);
        attr_headers_it++;
        if (view.size()) {
            result.emplace_back(view);
            total_size += view.size();
            if ((view.size() & 0x3) != 0) {
                const size_t padding_size = 4 - (view.size() % 4);
                result.emplace_back(util::ConstBinaryView(&padding, padding_size));
                total_size += padding_size;
            }
        }
    };

    std::optional<util::ByteVec> xor_mapped_data;
    std::optional<util::ByteVec> mapped_data;
    uint32_t prio = 0;
    uint64_t ice_controlling = 0;
    uint64_t ice_controlled = 0;
    std::optional<util::ByteVec> error_code_data;
    std::optional<util::ByteVec> unknown_attributes_data;
    std::optional<util::ByteVec> altenate_server_data;

    for (const auto& p: m_map) {
        const auto type = p.first.value();
        std::visit(
            util::overloaded {
                [&](const UsernameAttribute& a) {
                    const auto& username = a.name.value;
                    add_attr(type, util::ConstBinaryView(username.data(), username.size()));
                },
                [&](const SoftwareAttribute& a) {
                    add_attr(type, util::ConstBinaryView(a.name.data(), a.name.size()));
                },
                [&](const XorMappedAddressAttribute& a) {
                    xor_mapped_data = a.build();
                    add_attr(type, util::ConstBinaryView(*xor_mapped_data));
                },
                [&](const MappedAddressAttribute& a) {
                    mapped_data = a.build();
                    add_attr(type, util::ConstBinaryView(*mapped_data));
                },
                [&](const PriorityAttribute& a) {
                    prio = util::host_to_network_u32(a.priority);
                    add_attr(type, util::ConstBinaryView(&prio, sizeof(prio)));
                },
                [&](const IceControllingAttribute& a) {
                    ice_controlling = util::host_to_network_u64(a.tiebreaker);
                    add_attr(type, util::ConstBinaryView(&ice_controlling, sizeof(ice_controlling)));
                },
                [&](const IceControlledAttribute& a) {
                    ice_controlled = util::host_to_network_u64(a.tiebreaker);
                    add_attr(type, util::ConstBinaryView(&ice_controlled, sizeof(ice_controlled)));
                },
                [&](const UseCandidateAttribute&) {
                    add_attr(type, util::ConstBinaryView(&padding, 0));
                },
                [&](const ErrorCodeAttribute& ec) {
                    error_code_data = ec.build();
                    add_attr(type, util::ConstBinaryView(*error_code_data));
                },
                [&](const UnknownAttributesAttribute& a) {
                    unknown_attributes_data = a.build();
                    add_attr(type, util::ConstBinaryView(*unknown_attributes_data));
                },
                [&](const AlternateServerAttribute& a) {
                    altenate_server_data = a.build();
                    add_attr(type, util::ConstBinaryView(*altenate_server_data));
                },
                [&](const MessageIntegityAttribute&) { /* Do not add integrity here */ },
                [&](const FingerprintAttribute&) { /* Do not add fingerprint here */ }
            },
            p.second.value());
    }

    for (const auto& unknown: m_unknown) {
        add_attr(unknown.type.value(), util::ConstBinaryView(unknown.data));
    }

    // Add message integrity with dummy content (if exist in m_map);
    std::optional<Result<MessageIntegityAttribute::Digest>> maybe_integrity_digest_rv;
    if (maybe_integrity.has_value()) {
        const auto& h = maybe_integrity->hash;
        const auto& p = maybe_integrity->password;
        auto fake_header = header.build(total_size + details::STUN_ATTR_HEADER_SIZE + crypto::SHA1Hash::size);
        result[0] = util::ConstBinaryView(fake_header);
        maybe_integrity_digest_rv = crypto::hmac::digest(result, p.opad(), p.ipad(), h);
        auto maybe_err = maybe_integrity_digest_rv.value()
            .fmap([&](auto&& integrity_digest) {
                add_attr(attr_registry::MESSAGE_INTEGRITY, util::ConstBinaryView(integrity_digest.value.value()));
                return Unit{};
            });
        if (maybe_err.is_err()) {
            // Just adjust return type (fmap function never called);
            return maybe_err.fmap([](auto&&) { return util::ByteVec{}; });
        }
    }

    util::ByteVec real_header;
    uint32_t fingerprint_data = 0;
    if (m_map.find(AttributeType::from_uint16(attr_registry::FINGERPRINT)) != m_map.end()) {
        static_assert(sizeof(fingerprint_data) == details::FINGERPRINT_CRC_SIZE);
        real_header = header.build(total_size + details::FINGERPRINT_CRC_SIZE + details::STUN_ATTR_HEADER_SIZE);
        result[0] = util::ConstBinaryView(real_header);
        const auto fp = crc32(result) ^ FINGERPRINT_XOR;
        fingerprint_data = util::host_to_network_u32(fp);
        add_attr(attr_registry::FINGERPRINT, util::ConstBinaryView(&fingerprint_data, sizeof(fingerprint_data)));
    } else {
        real_header = header.build(total_size);
        result[0] = util::ConstBinaryView(real_header);
    }
    return util::ConstBinaryView::concat(result);
}

}
