//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Network path hash (from source to target)
//

#pragma once

#include <cstddef>
#include "net/net_path.hpp"
#include "util/util_hash_murmur.hpp"

namespace freewebrtc::net {

struct PathHash {
    std::size_t operator()(const net::Path&) const noexcept;
};

//
// inlines
//
inline std::size_t PathHash::operator()(const net::Path& path) const noexcept {
    std::array<util::ConstBinaryView, 2> arr { path.source.view(), path.target.view() };
    static constexpr uint64_t murmur_seed = 0;
    return util::hash::murmur<2>(arr, murmur_seed);
}

}


