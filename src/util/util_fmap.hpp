//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Some functional-like stuff for standard containers
//

#pragma once

#include <optional>

namespace freewebrtc::util {

// This is analogue of std::optional<T>::transform in C++23
template<typename T, typename Fmap>
auto fmap(const std::optional<T>&, Fmap&& f) -> std::optional<decltype(f(std::declval<T>()))>;

//
// implementation
//
template<typename T, typename Fmap>
auto fmap(const std::optional<T>& v, Fmap&& f) -> std::optional<decltype(f(std::declval<T>()))> {
    if (!v.has_value()) {
        return std::nullopt;
    }
    return f(v.value());
}

}
