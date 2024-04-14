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
    using Value = T;

    Maybe(T&& some);
    Maybe(const T&);
    Maybe(const None&);
    Maybe(Maybe&&) = default;
    Maybe(const Maybe&) = default;
    Maybe& operator=(const Maybe&) = default;
    Maybe& operator=(Maybe&&) = default;

    static Maybe<T> none();
    static Maybe<T> move_from(T&& some);
    static Maybe<T> copy_from(const T& some);

    T& unwrap() &;
    const T& unwrap() const&;
    T&& unwrap() &&;

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

    template<typename F>
    void with_inner(F&& f) &;
    template<typename F>
    void with_inner(F&& f) const&;
    template<typename F>
    void with_inner(F&& f) &&;

    template<typename F>
    auto bind(F&& f) & -> Maybe<typename std::invoke_result_t<F, T&>::Value>;
    template<typename F>
    auto bind(F&& f) const& -> Maybe<typename std::invoke_result_t<F, const T&>::Value>;
    template<typename F>
    auto bind(F&& f) && -> Maybe<typename std::invoke_result_t<F, T&&>::Value>;

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
inline Maybe<T> Maybe<T>::move_from(T&& some) {
    return Maybe<T>{std::move(some)};
}

template<typename T>
inline Maybe<T> Maybe<T>::copy_from(const T& some) {
    return Maybe<T>{some};
}

template<typename T>
Maybe<T> Maybe<T>::none() {
    return None{};
}

template<typename T>
inline T& Maybe<T>::unwrap() & {
    return std::get<T>(m_value);
}

template<typename T>
inline T&& Maybe<T>::unwrap() && {
    return std::get<T>(std::move(m_value));
}

template<typename T>
const T& Maybe<T>::unwrap() const & {
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
    m_value = None{};
    return v;
}

template<typename T>
template<typename F>
auto Maybe<T>::fmap(F&& f) & -> Maybe<std::invoke_result_t<F, T&>> {
    using ResultT = Maybe<std::invoke_result_t<F, T&&>>;
    if (is_none()) {
        return ResultT::none();
    }
    return ResultT::move_from(f(unwrap()));
}

template<typename T>
template<typename F>
auto Maybe<T>::fmap(F&& f) const& -> Maybe<std::invoke_result_t<F, const T&>> {
    using ResultT = Maybe<std::invoke_result_t<F, T&&>>;
    if (is_none()) {
        return ResultT::none();
    }
    return ResultT::move_from(f(unwrap()));
}

template<typename T>
template<typename F>
auto Maybe<T>::fmap(F&& f) && -> Maybe<std::invoke_result_t<F, T&&>> {
    using ResultT = Maybe<std::invoke_result_t<F, T&&>>;
    if (is_none()) {
        return ResultT::none();
    }
    return ResultT::move_from(f(std::get<T>(std::move(m_value))));
}

template<typename T>
template<typename F>
inline void Maybe<T>::with_inner(F&& f) & {
    if (is_none()) {
        return;
    }
    f(unwrap());
}

template<typename T>
template<typename F>
void Maybe<T>::with_inner(F&& f) const& {
    if (is_none()) {
        return;
    }
    f(unwrap());
}

template<typename T>
template<typename F>
void Maybe<T>::with_inner(F&& f) && {
    if (is_none()) {
        return;
    }
    f(std::get<Value>(std::move(m_value)));
}

template<typename T>
template<typename F>
auto Maybe<T>::bind(F&& f) & -> Maybe<typename std::invoke_result_t<F, T&>::Value> {
    using ResultT = Maybe<typename std::invoke_result_t<F, T&>::Value>;
    if (is_none()) {
        return ResultT::none();
    }
    return f(unwrap());
}

template<typename T>
template<typename F>
auto Maybe<T>::bind(F&& f) const& -> Maybe<typename std::invoke_result_t<F, const T&>::Value> {
    using ResultT = Maybe<typename std::invoke_result_t<F, const T&>::Value>;
    if (is_none()) {
        return ResultT::none();
    }
    return f(unwrap());
}

template<typename T>
template<typename F>
auto Maybe<T>::bind(F&& f) && -> Maybe<typename std::invoke_result_t<F, T&&>::Value> {
    using ResultT = Maybe<typename std::invoke_result_t<F, T&&>::Value>;
    if (is_none()) {
        return ResultT::none();
    }
    return f(std::get<Value>(std::move(m_value)));
}


inline None none() noexcept {
    return None{};
}


}
