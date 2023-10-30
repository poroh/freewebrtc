//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// HMAC implementation as defined here:
// https://www.rfc-editor.org/rfc/rfc2104
//

#pragma once

#include <cstdint>

#include "util/util_binary_view.hpp"
#include "util/util_tagged_type.hpp"
#include "util/util_return_value.hpp"
#include "util/util_flat.hpp"
#include "crypto/crypto_hash.hpp"

namespace freewebrtc::crypto::hmac {

template<uint8_t xorv>
class PadKey {
public:
    template<typename HashFunc>
    static ReturnValue<PadKey> from_key(const util::ConstBinaryView&, HashFunc h);

    PadKey(const PadKey&) = default;
    PadKey(PadKey&&) = default;
    PadKey& operator=(const PadKey&) = default;
    PadKey& operator=(PadKey&&) = default;
    bool operator==(const PadKey&) const noexcept = default;

    util::ConstBinaryView view() const noexcept;
private:
    static constexpr size_t B = 64;
    using Data = std::array<uint8_t, B>;
    PadKey(Data&&);
    std::array<uint8_t, B> m_data;
};

using IPadKey = PadKey<0x36>;
using OPadKey = PadKey<0x5C>;

template<typename Hash>
using Digest = util::TaggedType<Hash>;

template<typename HashFunc>
using HMACReturnValue = ReturnValue<Digest<typename HashFunc::result_type::Value>>;

template<typename HashFunc>
HMACReturnValue<HashFunc> digest(const std::vector<util::ConstBinaryView>& data, const OPadKey& opad, const IPadKey& ipad, HashFunc h);

//
// implementation
//
template<uint8_t xorv>
inline PadKey<xorv>::PadKey(Data&& d)
    : m_data(std::move(d))
{
    for (auto& v: m_data) {
        v ^= xorv;
    }
}

template<uint8_t xorv>
template<typename HashFunc>
inline ReturnValue<PadKey<xorv>> PadKey<xorv>::from_key(const util::ConstBinaryView& view, HashFunc h)
{
    util::ConstBinaryView v = view;
    if (v.size() > B) {
        auto hash = h({view});
        if (auto maybe_err = hash.error(); maybe_err.has_value()) {
            return *maybe_err;
        }
        v = hash.value()->get().view();
    }
    Data data = {0};
    std::copy(v.begin(), v.end(), data.begin());
    return PadKey<xorv>(std::move(data));
}

template<uint8_t xorv>
inline util::ConstBinaryView PadKey<xorv>::view() const noexcept {
    return util::ConstBinaryView(m_data);
}

template<typename HashFunc>
HMACReturnValue<HashFunc> digest(const std::vector<util::ConstBinaryView>& data, const OPadKey& opad, const IPadKey& ipad, HashFunc h) {
    std::vector<util::ConstBinaryView> inner_data = {ipad.view()};
    std::copy(data.begin(), data.end(), std::back_inserter(inner_data));
    auto inner = h(inner_data);
    if (inner.error().has_value()) {
        return *inner.error();
    }
    auto outer = h({opad.view(), inner.value()->get().view()});
    if (outer.error().has_value()) {
        return *outer.error();
    }
    return (typename HMACReturnValue<HashFunc>::Value)(std::move(*outer.value()));
}

}
