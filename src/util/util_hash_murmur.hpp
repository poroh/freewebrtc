//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Murmur hash implementation
//

#pragma once

#include <cstddef>
#include "util/util_binary_view.hpp"

namespace freewebrtc::util::hash {

std::size_t murmur(const ConstBinaryView& view, uint64_t seed) noexcept;

template<int sz>
std::size_t murmur(const std::array<ConstBinaryView, sz>& views, uint64_t seed) noexcept;

//
// inlines
//
template<int sz>
inline std::size_t murmur(const std::array<ConstBinaryView, sz>& views, uint64_t seed) noexcept {
    for (size_t i = 0; i < sz; ++i) {
        seed = murmur(views[i], seed);
    }
    return seed;
}


}



