//
// Copyright (c) 2024 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Maybe (better replacement of the std::optional)
// Motivation of the introduction limited version of optional
// is that std::optional is not safe for development and has
// some not safe properties like:
// 1. implicit conversion to boolean
// 2. implicit creation from value
// 3. unsafe getting value without unwrapping
// 4. implicit comparisons for None values
//
// Plus it does not have good functor / monadic functions
// 1. fmap
// 2. bind
//

#pragma once

#include <variant>
#include <type_traits>

#include "util/util_tagged_type.hpp"
#include "util/util_unit.hpp"
#include "util/util_result.hpp"
#include "util/util_error_code.hpp"

namespace freewebrtc {

struct NoneTag{};
using None = util::TaggedType<Unit, NoneTag>;

template<typename T>
class Maybe {
public:
    Maybe(T&& some);
    Maybe(const T&);
    Maybe(const None&);
    Maybe(Maybe&&) = default;
    Maybe(const Maybe&) = default;
    Maybe& operator=(const Maybe&) = default;
    Maybe& operator=(Maybe&&) = default;

    T& unwrap();
    const T& unwrap() const;

    const T& value_or(const T&) const;

    template<typename F>
    auto value_or_call(F&& f) -> std::invoke_result_t<F>;

    bool is_some() const;
    bool is_none() const;

    Result<T> require();

    template<typename F>
    auto fmap(F&& f) & -> Maybe<std::invoke_result_t<F, T&>>;
    template<typename F>
    auto fmap(F&& f) const& -> Maybe<std::invoke_result_t<F, const T&>>;
    template<typename F>
    auto fmap(F&& f) && -> Maybe<std::invoke_result_t<F, T&&>>;


private:
    std::variant<T, None> m_value;
};

None none() noexcept;

//
// implementation
//
template<typename T>
inline Maybe<T>::Maybe(const T& t)
    : m_value(t)
{}

template<typename T>
inline Maybe<T>::Maybe(T&& t)
    : m_value(std::move(t))
{}

template<typename T>
inline Maybe<T>::Maybe(const None& n)
    : m_value(n)
{}


template<typename T>
inline T& Maybe<T>::unwrap() {
    return std::get<T>(m_value);
}

template<typename T>
const T& Maybe<T>::unwrap() const {
    return std::get<T>(m_value);
}

template<typename T>
const T& Maybe<T>::value_or(const T& dflt) const {
    if (is_none()) {
        return dflt;
    }
    return unwrap();
}

template<typename T>
template<typename F>
auto Maybe<T>::value_or_call(F&& f) -> std::invoke_result_t<F> {
    static_assert(std::is_same_v<T, std::remove_cvref_t<std::invoke_result_t<F>>>);
    if (is_none()) {
        return f();
    }
    return unwrap();
}

template<typename T>
inline bool Maybe<T>::is_some() const {
    return std::holds_alternative<T>(m_value);
}

template<typename T>
inline bool Maybe<T>::is_none() const {
    return std::holds_alternative<None>(m_value);
}

template<typename T>
Result<T> Maybe<T>::require() {
    if (is_none()) {
        return make_error_code(util::ErrorCode::value_required_in_maybe);
    }
    auto v = std::move(unwrap());
    m_value = none();
    return v;
}

template<typename T>
template<typename F>
auto Maybe<T>::fmap(F&& f) & -> Maybe<std::invoke_result_t<F, T&>> {
    if (is_none()) {
        return none();
    }
    using ResultT = std::invoke_result_t<F, T&&>;
    return Maybe<ResultT>{f(unwrap())};
}

template<typename T>
template<typename F>
auto Maybe<T>::fmap(F&& f) const& -> Maybe<std::invoke_result_t<F, const T&>> {
    if (is_none()) {
        return none();
    }
    using ResultT = std::invoke_result_t<F, T&&>;
    return Maybe<ResultT>{f(unwrap())};
}

template<typename T>
template<typename F>
auto Maybe<T>::fmap(F&& f) && -> Maybe<std::invoke_result_t<F, T&&>> {
    if (is_none()) {
        return none();
    }
    using ResultT = std::invoke_result_t<F, T&&>;
    return Maybe<ResultT>{f(std::get<T>(std::move(m_value)))};
}

inline None none() noexcept {
    return None{};
}


}
