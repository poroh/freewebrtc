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

namespace freewebrtc {

// Syntax sugar for Function fmap:
template<typename V, typename F>
ReturnValue<std::invoke_result_t<F, V&>> operator>=(ReturnValue<V>&, F&& f);
template<typename V, typename F>
ReturnValue<std::invoke_result_t<F, const V&>> operator>=(const ReturnValue<V>&, F&& f);
template<typename V, typename F>
ReturnValue<std::invoke_result_t<F, V&&>> operator>=(ReturnValue<V>&&, F&& f);


// Syntax sugar for Mondaic bind:
template<typename V, typename F>
ReturnValue<typename std::invoke_result_t<F, V&>::Value> operator>(ReturnValue<V>&, F&& f);
template<typename V, typename F>
ReturnValue<typename std::invoke_result_t<F, const V&>::Value> operator>(const ReturnValue<V>&, F&& f);
template<typename V, typename F>
ReturnValue<typename std::invoke_result_t<F, V&&>::Value> operator>(ReturnValue<V>&&, F&& f);

//
// implementation
//
template<typename V, typename F>
ReturnValue<std::invoke_result_t<F, V&>> operator>=(ReturnValue<V>& v, F&& f) {
    return v.fmap(f);
}

template<typename V, typename F>
ReturnValue<std::invoke_result_t<F, const V&>> operator>=(const ReturnValue<V>& v, F&& f) {
    return v.fmap(f);
}

template<typename V, typename F>
ReturnValue<std::invoke_result_t<F, V&&>> operator>=(ReturnValue<V>&& v, F&& f) {
    return std::move(v).fmap(f);
}

template<typename V, typename F>
ReturnValue<typename std::invoke_result_t<F, V&>::Value> operator>(ReturnValue<V>& v, F&& f) {
    return v.bind(f);
}

template<typename V, typename F>
ReturnValue<typename std::invoke_result_t<F, const V&>::Value> operator>(const ReturnValue<V>& v, F&& f) {
    return v.bind(f);
}

template<typename V, typename F>
ReturnValue<typename std::invoke_result_t<F, V&&>::Value> operator>(ReturnValue<V>&& v, F&& f) {
    return std::move(v).bind(f);
}

}
