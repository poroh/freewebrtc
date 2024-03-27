//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Definitions of types of functions that calculates hashes
// Freewebrtc may use different providers of these crypto
// functions (it is cusomizable)
//

#pragma once

#include <vector>
#include <array>
#include <system_error>
#include <functional>

#include "util/util_binary_view.hpp"
#include "util/util_result.hpp"

namespace freewebrtc::crypto {

template<typename TagType, size_t SIZE>
class Hash {
public:
    using Tag = TagType;
    using Value = std::array<uint8_t, SIZE>;
    using Input = std::vector<util::ConstBinaryView>;
    using Result = ::freewebrtc::Result<Hash>;
    using Func = std::function<Result(const Input&)>;

    static constexpr auto size = SIZE;

    static std::optional<Hash> from_view(const util::ConstBinaryView&);

    explicit Hash(Value&&);


    Hash(const Hash&) = default;
    Hash(Hash&&) = default;
    Hash& operator=(const Hash&) = default;

    const Value& value() const noexcept;
    bool operator==(const Hash&) const noexcept;

    util::ConstBinaryView view() const noexcept;
private:
    std::array<uint8_t, SIZE> m_data;
};

struct MD5HashTag{};
struct SHA1HashTag{};
struct SHA256HashTag{};

using MD5Hash = Hash<MD5HashTag, 16>;
using SHA1Hash = Hash<SHA1HashTag, 20>;
using SHA256Hash = Hash<SHA256HashTag, 32>;

//
// implementation
//
template<typename TagType, size_t SIZE>
std::optional<Hash<TagType, SIZE>> Hash<TagType, SIZE>::from_view(const util::ConstBinaryView& vv) {
    if (vv.size() != size) {
        return std::nullopt;
    }
    Value v;
    std::copy(vv.begin(), vv.end(), v.begin());
    return Hash(std::move(v));
}

template<typename TagType, size_t SIZE>
inline Hash<TagType, SIZE>::Hash(Value&& v)
    : m_data(std::move(v))
{}

template<typename TagType, size_t SIZE>
const typename Hash<TagType, SIZE>::Value& Hash<TagType, SIZE>::value() const noexcept {
    return m_data;
}

template<typename TagType, size_t SIZE>
inline bool Hash<TagType, SIZE>::operator==(const Hash& other) const noexcept {
    return m_data == other.m_data;
}

template<typename TagType, size_t SIZE>
inline util::ConstBinaryView Hash<TagType, SIZE>::view() const noexcept {
    return util::ConstBinaryView(m_data);
}


}
