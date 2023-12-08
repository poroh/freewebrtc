//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Operators overload to reduce number of parenthesis
// in code
//

#pragma once

#include <type_traits>
#include "util/util_return_value.hpp"
#include "util/util_tagged_type.hpp"

namespace freewebrtc {

template<typename T> T strip_rv(ReturnValue<T>&);
template<typename T> T strip_rv(const ReturnValue<T>&);
template<typename T> T strip_rv(ReturnValue<T>&&);
template<typename T> T strip_rv(T&&);
template<typename T> T strip_rv(T&);

// Syntax sugar for Mondaic bind:
template<typename V, typename F>
auto operator>(ReturnValue<V>&, F&& f);
template<typename V, typename F>
auto operator>(const ReturnValue<V>&, F&& f);
template<typename V, typename F>
auto operator>(ReturnValue<V>&&, F&& f);

//
// implementation
//
template<typename V, typename F>
auto operator>(ReturnValue<V>& v, F&& f) {
    using RetV = std::invoke_result_t<F, V&>;
    using StrippedV = decltype(strip_rv(f(std::declval<V&>())));
    if constexpr (std::is_same_v<RetV, StrippedV>) {
        return v.fmap(f);
    } else {
        return v.bind(f);
    }
}

template<typename V, typename F>
auto operator>(const ReturnValue<V>& v, F&& f) {
    using RetV = std::invoke_result_t<F, V&>;
    using StrippedV = decltype(strip_rv(f(std::declval<const V&>())));
    if constexpr (std::is_same_v<RetV, StrippedV>) {
        return v.fmap(f);
    } else {
        return v.bind(f);
    }
}

template<typename V, typename F>
auto operator>(ReturnValue<V>&& v, F&& f) {
    using RetV = std::invoke_result_t<F, V&&>;
    using StrippedV = decltype(strip_rv(f(std::declval<V&&>())));
    if constexpr (std::is_same_v<RetV, StrippedV>) {
        return std::move(v).fmap(f);
    } else {
        return std::move(v).bind(f);
    }
}

}
