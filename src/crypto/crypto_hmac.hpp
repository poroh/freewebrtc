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
#include "util/util_result.hpp"
#include "util/util_flat.hpp"
#include "crypto/crypto_hash.hpp"

namespace freewebrtc::crypto::hmac {

template<uint8_t xorv>
class PadKey {
public:
    template<typename HashFunc>
    static Result<PadKey> from_key(const util::ConstBinaryView&, HashFunc h);

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
using HMACResult = Result<Digest<typename HashFunc::result_type::Value>>;

template<typename HashFunc>
HMACResult<HashFunc> digest(const std::vector<util::ConstBinaryView>& data, const OPadKey& opad, const IPadKey& ipad, HashFunc h);

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
inline Result<PadKey<xorv>> PadKey<xorv>::from_key(const util::ConstBinaryView& view, HashFunc h)
{
    using ViewRV = Result<util::ConstBinaryView>;
    return ViewRV(view)
        .bind([&](auto&& v) -> ViewRV {
            if (v.size() > B) {
                // TODO: is result of v.view is valid after exit from this function?
                return h({v}).fmap([](auto&& v) { return v.view(); });
            } else {
                return v;
            }
        })
        .fmap([](auto&& v) {
            Data data = {0};
            std::copy(v.begin(), v.end(), data.begin());
            return PadKey<xorv>(std::move(data));
        });
}

template<uint8_t xorv>
inline util::ConstBinaryView PadKey<xorv>::view() const noexcept {
    return util::ConstBinaryView(m_data);
}

template<typename HashFunc>
HMACResult<HashFunc> digest(const std::vector<util::ConstBinaryView>& data, const OPadKey& opad, const IPadKey& ipad, HashFunc h) {
    using HashFuncInput = std::vector<util::ConstBinaryView>;
    HashFuncInput inner_data = {ipad.view()};
    inner_data.reserve(data.size() + 1);
    std::copy(data.begin(), data.end(), std::back_inserter(inner_data));
    using HashFuncResult = std::invoke_result_t<HashFunc, HashFuncInput>;
    using HashValue = typename HashFuncResult::Value;
    using ResultT = typename HMACResult<HashFunc>::Value;
    return h(inner_data)
        .bind([&](HashValue&& inner) {
            // TODO: is result of inner.view is valid after exit from this function?
            return h({opad.view(), inner.view()});
        })
        .fmap(ResultT::move_from);
}

}
