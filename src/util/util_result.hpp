//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Helper to handle either return value or std::error_code
//

#pragma once

#include <variant>
#include <tuple>
#include <system_error>
#include <type_traits>
#include <utility>

#include "util/util_error.hpp"
#include "util/util_unit.hpp"

namespace freewebrtc {

template<typename V>
class Result {
public:
    using Self = Result<V>;
    using Value = V;
    using Error = ::freewebrtc::Error;

    Result(Value&&);
    Result(const Value&);
    Result(std::error_code&&);
    Result(Error&&);
    Result(const std::error_code&);
    Result(const Error&);
    Result(const Result&) = default;
    Result(Result&&) = default;
    Result& operator=(const Result&) = default;
    Result& operator=(Result&&) = default;

    bool is_err() const noexcept;
    const Error& unwrap_err() const noexcept;

    bool is_ok() const noexcept;
    Value& unwrap() noexcept;
    const Value& unwrap() const noexcept;
    const Value& unwrap_or(const Value&) const noexcept;

    template<typename F>
    auto value_or_call(F&& f) -> std::invoke_result_t<F, const Error&>;

    // Functor's fmap
    // Apply function to Value if it Result is value
    // and return result as Result of result of the function.
    template<typename F>
    auto fmap(F&& f) & -> Result<std::invoke_result_t<F, V&>>;
    template<typename F>
    auto fmap(F&& f) const& -> Result<std::invoke_result_t<F, const V&>>;
    template<typename F>
    auto fmap(F&& f) && -> Result<std::invoke_result_t<F, V&&>>;

    // Monadic binding:
    // F must return Result<SomeType>
    // f is called only if this Result is not error
    template<typename F>
    auto bind(F&& f) & -> Result<typename std::invoke_result_t<F, V&>::Value>;
    template<typename F>
    auto bind(F&& f) const& -> Result<typename std::invoke_result_t<F, const V&>::Value>;
    template<typename F>
    auto bind(F&& f) && -> Result<typename std::invoke_result_t<F, V&&>::Value>;

    // Bind error alternative
    template<typename F>
    Self bind_err(F&&) &;
    template<typename F>
    Self bind_err(F&&) const&;
    template<typename F>
    Self bind_err(F&&) &&;

    // Context to error (if Result contains error);
    template<typename... Ts>
    Self& add_context(Ts&&...);

private:
    std::variant<Value, Error> m_result;
};

template<typename T>
Result<T> success(T&&) noexcept;

using MaybeError = Result<Unit>;
MaybeError success() noexcept;

// Create Result from the type. In FP it is full anaglogue of return
// for Monad or pure for applicative functor.
template<typename T>
Result<T> return_value(T&& t) noexcept;

// Check that any of the Result is error.
// Example:
//   Result<int> v1 = 1;
//   Result<std::string> v2 = "1234";
//   Result<double> v3 = std::make_error_code(std::errc::invalid_argument);
//   assert(any_is_err(v1, v2, v3));
template<typename T, typename... Ts>
MaybeError any_is_err(const Result<T>& first, const Result<Ts>&... rest);

// Combine Result s with function
// Example:
//   Result<int> v1 = 1;
//   Result<std::string> v2 = "1234";
//   Result<double> v3 = 0.2;
//   combine([](int v1, const std::string& v2, double v3) {
//       // .. some code
//   },
//   v1, v2, v3)
template<typename F, typename... Ts>
auto combine(F&& f, const Result<Ts>&... rvs) -> Result<typename std::invoke_result_t<F, const Ts&...>::Value>;
template<typename F, typename... Ts>
auto combine(F&& f, Result<Ts>&... rvs) -> Result<typename std::invoke_result_t<F, Ts&...>::Value>;
template<typename F, typename... Ts>
auto combine(F&& f, Result<Ts>&&... rvs) -> Result<typename std::invoke_result_t<F, Ts&&...>::Value>;

//
// inlines
//
template<typename V>
inline Result<V>::Result(const Value& v)
    : m_result(v)
{}

template<typename V>
inline Result<V>::Result(Value&& v)
    : m_result(std::move(v))
{}

template<typename V>
inline Result<V>::Result(std::error_code&& c)
    : m_result(std::move(c))
{}

template<typename V>
inline Result<V>::Result(const std::error_code& c)
    : m_result(c)
{}

template<typename V>
inline Result<V>::Result(Error&& e)
    : m_result(std::move(e))
{}

template<typename V>
inline Result<V>::Result(const Error& e)
    : m_result(e)
{}

template<typename V>
inline bool Result<V>::is_err() const noexcept {
    return std::holds_alternative<Error>(m_result);
}

template<typename V>
inline const typename Result<V>::Error&
Result<V>::unwrap_err() const noexcept {
    return std::get<Error>(m_result);
}

template<typename V>
inline bool Result<V>::is_ok() const noexcept {
    return std::holds_alternative<Value>(m_result);
}

template<typename V>
inline const typename Result<V>::Value&
Result<V>::unwrap() const noexcept {
    return std::get<Value>(m_result);
}

template<typename V>
inline typename Result<V>::Value&
Result<V>::unwrap() noexcept {
    return std::get<Value>(m_result);
}

template<typename V>
inline const typename Result<V>::Value&
Result<V>::unwrap_or(const V& v) const noexcept {
    if (is_ok()) {
        return unwrap();
    }
    return v;
}

template<typename V>
template<typename F>
auto Result<V>::fmap(F&& f) & -> Result<std::invoke_result_t<F, V&>> {
    if (is_err()) {
        return unwrap_err();
    }
    return f(unwrap());
}

template<typename V>
template<typename F>
auto Result<V>::fmap(F&& f) const& -> Result<std::invoke_result_t<F, const V&>> {
    if (is_err()) {
        return unwrap_err();
    }
    return f(unwrap());
}

template<typename V>
template<typename F>
auto Result<V>::fmap(F&& f) && -> Result<std::invoke_result_t<F, V&&>> {
    if (is_err()) {
        return unwrap_err();
    }
    return f(std::get<Value>(std::move(m_result)));
}

template<typename V>
template<typename F>
auto Result<V>::bind(F&& f) & -> Result<typename std::invoke_result_t<F, V&>::Value> {
    if (is_err()) {
        return unwrap_err();
    }
    return f(unwrap());
}

template<typename V>
template<typename F>
auto Result<V>::bind(F&& f) const& -> Result<typename std::invoke_result_t<F, const V&>::Value> {
    if (is_err()) {
        return unwrap_err();
    }
    return f(unwrap());
}

template<typename V>
template<typename F>
auto Result<V>::bind(F&& f) && -> Result<typename std::invoke_result_t<F, V&&>::Value> {
    if (is_err()) {
        return unwrap_err();
    }
    return f(std::get<Value>(std::move(m_result)));
}

template<typename V>
template<typename F>
Result<V> Result<V>::bind_err(F&& f) & {
    if (is_err()) {
        return f(unwrap_err());
    }
    return *this;
}

template<typename V>
template<typename F>
Result<V> Result<V>::bind_err(F&& f) const& {
    if (is_err()) {
        return f(unwrap_err());
    }
    return *this;
}

template<typename V>
template<typename F>
Result<V> Result<V>::bind_err(F&& f) && {
    if (is_err()) {
        return f(unwrap_err());
    }
    return *this;
}

template<typename T>
template<typename F>
auto Result<T>::value_or_call(F&& f) -> std::invoke_result_t<F, const Error&> {
    static_assert(std::is_same_v<T, std::remove_cvref_t<std::invoke_result_t<F, const Error&>>>);
    if (is_err()) {
        return f(unwrap_err());
    }
    return unwrap();
}

template<typename V>
template<typename... Ts>
Result<V>& Result<V>::add_context(Ts&&... v) {
    if (!is_err()) {
        return *this;
    }
    std::get<Error>(m_result).add_context(std::forward<Ts>(v)...);
    return *this;
}

template<typename T, typename... Ts>
MaybeError any_is_err(const Result<T>& first, const Result<Ts>&... rest) {
    if (first.is_err()) {
        return first.unwrap_err();
    }
    if constexpr (sizeof...(rest) > 0) {
        return any_is_err(rest...);
    } else {
        return success();
    }
}

template<typename Func, typename... Vs>
auto combine_with_values_cref_hh(Func func, std::tuple<const Vs&...>&& values) {
    return std::apply(func, std::move(values));
}

template<typename F, typename... Vs, typename T, typename... Ts>
auto combine_with_values_cref_h(F func, std::tuple<const Vs&...>&& values, const Result<T>& rv, const Result<Ts>&... rest) {
    std::tuple<const T&> v{rv.unwrap()};
    auto next_tuple = std::tuple_cat(std::move(values), v);
    if constexpr (sizeof...(rest) > 0) {
        return combine_with_values_cref_h(func, std::move(next_tuple), std::forward<const Result<Ts>&>(rest)...);
    } else {
        return combine_with_values_cref_hh(func, std::move(next_tuple));
    }
}

// Base function to start the process with an empty tuple
template<typename Func, typename... Ts>
auto combine_with_values_cref(Func func, const Result<Ts>&... rvs) {
    return combine_with_values_cref_h(func, std::tuple<>{}, std::forward<const Result<Ts>&>(rvs)...);
}

template<typename F, typename... Ts>
auto combine(F&& f, const Result<Ts>&... rvs) -> Result<typename std::invoke_result_t<F, const Ts&...>::Value> {
    if (auto maybe_error = any_is_err(rvs...); maybe_error.is_err()) {
        return maybe_error.unwrap_err();
    }
    auto result = combine_with_values_cref(f, std::forward<const Result<Ts>&>(rvs)...);
    if (result.is_err()) {
        return result.unwrap_err();
    }
    return result.unwrap();
}

template<typename Func, typename... Vs>
auto combine_with_values_ref_hh(Func func, std::tuple<Vs&...>&& values) {
    return std::apply(func, std::move(values));
}

template<typename F, typename... Vs, typename T, typename... Ts>
auto combine_with_values_ref_h(F func, std::tuple<Vs&...>&& values, Result<T>& rv, Result<Ts>&... rest) {
    std::tuple<T&> v{rv.unwrap()};
    auto next_tuple = std::tuple_cat(std::move(values), v);
    if constexpr (sizeof...(rest) > 0) {
        return combine_with_values_ref_h(func, std::move(next_tuple), std::forward<Result<Ts>&>(rest)...);
    } else {
        return combine_with_values_ref_hh(func, std::move(next_tuple));
    }
}

// Base function to start the process with an empty tuple
template<typename Func, typename... Ts>
auto combine_with_values_ref(Func func, Result<Ts>&... rvs) {
    return combine_with_values_ref_h(func, std::tuple<>{}, std::forward<Result<Ts>&>(rvs)...);
}

template<typename F, typename... Ts>
auto combine(F&& f, Result<Ts>&... rvs) -> Result<typename std::invoke_result_t<F, Ts&...>::Value> {
    if (auto maybe_error = any_is_err(rvs...); maybe_error.is_err()) {
        return maybe_error.unwrap_err();
    }
    auto result = combine_with_values_ref(f, std::forward<Result<Ts>&>(rvs)...);
    if (result.is_err()) {
        return result.unwrap_err();
    }
    return std::move(result.unwrap());
}

template<typename F, typename... Values>
auto combine_rv_with_values_hh(F&& func, std::tuple<Values&&...>&& values) {
    return std::apply(func, std::move(values));
}

template<typename F, typename... Ts, typename T, typename... Rest>
auto combine_rv_with_values_h(F&& func, std::tuple<Ts&&...>&& values, Result<T>&& rv, Result<Rest>&&... rest) {
    std::tuple<T&&> v{std::move(rv.unwrap())};
    auto next_tuple = std::tuple_cat(std::move(values), std::move(v));
    if constexpr (sizeof...(rest) > 0) {
        return combine_rv_with_values_h(std::move(func), std::move(next_tuple), std::forward<Result<Rest>>(rest)...);
    } else {
        return combine_rv_with_values_hh(std::move(func), std::move(next_tuple));
    }
}

// Base function to start the process with an empty tuple
template<typename F, typename... Ts>
auto combine_rv_with_values(F&& func, Result<Ts>&&... rvs) {
    return combine_rv_with_values_h(std::move(func), std::tuple<>{}, std::forward<Result<Ts>>(rvs)...);
}

template<typename F, typename... Ts>
auto combine(F&& f, Result<Ts>&&... rvs) -> Result<typename std::invoke_result_t<F, Ts&&...>::Value>  {
    if (auto maybe_error = any_is_err(rvs...); maybe_error.is_err()) {
        return maybe_error.unwrap_err();
    }
    auto result = combine_rv_with_values(std::move(f), std::forward<Result<Ts>>(rvs)...);
    if (result.is_err()) {
        return result.unwrap_err();
    }
    return std::move(result.unwrap());
}

template<typename T>
Result<T> success(T&& v) noexcept {
    return Result<T>(std::move(v));
}

inline MaybeError success() noexcept {
    return MaybeError{Unit::create()};
}

template<typename T>
Result<T> return_value(T&& t) noexcept {
    return Result<T>(std::forward<T>(t));
}


}
