//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Helper to handle either return value or std::error_code
//

#pragma once

#include <variant>
#include <system_error>
#include <type_traits>
#include <optional>
#include <utility>

namespace freewebrtc {

template<typename T> class ReturnValue;

template<typename T> T strip_rv(const ReturnValue<T>& v);
template<typename T> T strip_rv(ReturnValue<T>&& v);
template<typename T> T strip_rv(const T& v);
template<typename T> T strip_rv(T&& v);

template<typename V>
class ReturnValue {
public:
    using Value = V;
    using Error = std::error_code;

    using MaybeError = std::optional<Error>;
    using MaybeValue = std::optional<std::reference_wrapper<Value>>;
    using MaybeConstValue = std::optional<std::reference_wrapper<const Value>>;

    ReturnValue(Value&&);
    ReturnValue(const Value&);
    ReturnValue(std::error_code&&);
    ReturnValue(const std::error_code&);
    ReturnValue(const ReturnValue&) = default;
    ReturnValue(ReturnValue&&) = default;
    ReturnValue& operator=(const ReturnValue&) = default;
    ReturnValue& operator=(ReturnValue&&) = default;

    bool is_error() const noexcept;
    MaybeError maybe_error() const noexcept;
    const Error& assert_error() const noexcept;

    bool is_value() const noexcept;
    MaybeValue maybe_value() noexcept;
    MaybeConstValue maybe_value() const noexcept;
    Value& assert_value() noexcept;
    const Value& assert_value() const noexcept;

    // Apply function to Value if it ReturnValue is value
    // and return result as ReturnValue of result of the function.
    // If function Fmap returns ReturnValue<SomeTime> then
    // fmap return also returns ReturnValue<SomeTime> instead of
    // ReturnValue<ReturnValue<SomeTime>>
    template<typename Fmap>
    auto fmap(Fmap&& f) & -> ReturnValue<decltype(strip_rv(f(std::declval<V&>())))>;
    template<typename Fmap>
    auto fmap(Fmap&& f) const& -> ReturnValue<decltype(strip_rv(f(std::declval<const V&>())))>;
    template<typename Fmap>
    auto fmap(Fmap&& f) && -> ReturnValue<decltype(strip_rv(f(std::declval<V&&>())))>;

private:
    std::variant<Value, Error> m_result;
};

// Check that any of the ReturnValue is error.
// Example:
//   ReturnValue<int> v1 = 1;
//   ReturnValue<std::string> v2 = "1234";
//   ReturnValue<double> v3 = std::make_error_code(std::errc::invalid_argument);
//   assert(any_is_error(v1, v2, v3));
template<typename T, typename... Ts>
std::optional<std::error_code> any_is_error(const ReturnValue<T>& first, const ReturnValue<Ts>&... rest);

// Combine ReturnValue s with function
// Example:
//   ReturnValue<int> v1 = 1;
//   ReturnValue<std::string> v2 = "1234";
//   ReturnValue<double> v3 = 0.2;
//   combine([](int v1, const std::string& v2, double v3) {
//       // .. some code
//   },
//   v1, v2, v3)
template<typename F, typename... Ts>
auto combine(F&& f, const ReturnValue<Ts>&... rvs) -> ReturnValue<decltype(strip_rv(f(std::declval<const Ts&>()...)))>;
template<typename F, typename... Ts>
auto combine(F&& f, ReturnValue<Ts>&... rvs) -> ReturnValue<decltype(strip_rv(f(std::declval<Ts&>()...)))>;
template<typename F, typename... Ts>
auto combine(F&& f, ReturnValue<Ts>&&... rvs) -> ReturnValue<decltype(strip_rv(f(std::declval<Ts&&>()...)))>;

//
// inlines
//
template<typename V>
inline ReturnValue<V>::ReturnValue(const Value& v)
    : m_result(v)
{}

template<typename V>
inline ReturnValue<V>::ReturnValue(Value&& v)
    : m_result(std::move(v))
{}

template<typename V>
inline ReturnValue<V>::ReturnValue(std::error_code&& c)
    : m_result(std::move(c))
{}

template<typename V>
inline ReturnValue<V>::ReturnValue(const std::error_code& c)
    : m_result(c)
{}


template<typename V>
inline bool ReturnValue<V>::is_error() const noexcept {
    return std::holds_alternative<Error>(m_result);
}

template<typename V>
inline std::optional<typename ReturnValue<V>::Error>
ReturnValue<V>::maybe_error() const noexcept {
    auto err = std::get_if<Error>(&m_result);
    return err != nullptr ? *err : std::optional<Error>(std::nullopt);
}

template<typename V>
inline const typename ReturnValue<V>::Error&
ReturnValue<V>::assert_error() const noexcept {
    return std::get<Error>(m_result);
}

template<typename V>
inline bool ReturnValue<V>::is_value() const noexcept {
    return std::holds_alternative<Value>(m_result);
}

template<typename V>
inline typename ReturnValue<V>::MaybeConstValue
ReturnValue<V>::maybe_value() const noexcept {
    auto v = std::get_if<V>(&m_result);
    return v != nullptr ? MaybeConstValue{*v} : std::nullopt;
}

template<typename V>
inline typename ReturnValue<V>::MaybeValue
ReturnValue<V>::maybe_value() noexcept {
    auto v = std::get_if<V>(&m_result);
    return v != nullptr ? MaybeValue{*v} : std::nullopt;
}

template<typename V>
inline const typename ReturnValue<V>::Value&
ReturnValue<V>::assert_value() const noexcept {
    return std::get<Value>(m_result);
}

template<typename V>
inline typename ReturnValue<V>::Value&
ReturnValue<V>::assert_value() noexcept {
    return std::get<Value>(m_result);
}

template<typename V>
template<typename Fmap>
auto ReturnValue<V>::fmap(Fmap&& f) & -> ReturnValue<decltype(strip_rv(f(std::declval<V&>())))> {
    using FRetT = decltype(f(std::declval<V&>()));
    using FmapRet = decltype(strip_rv(f(std::declval<V&>())));
    if (is_error()) {
        return assert_error();
    }
    if constexpr (std::is_same_v<FRetT, FmapRet>) {
        return f(assert_value());
    } else {
        auto result = f(assert_value());
        if (result.is_error()) {
            return result.assert_error();
        }
        return result.assert_value();
    }
}

template<typename V>
template<typename Fmap>
auto ReturnValue<V>::fmap(Fmap&& f) const& -> ReturnValue<decltype(strip_rv(f(std::declval<const V&>())))> {
    using FRetT = decltype(f(std::declval<const V&>()));
    using FmapRet = decltype(strip_rv(f(std::declval<const V&>())));
    if (is_error()) {
        return assert_error();
    }
    if constexpr (std::is_same_v<FRetT, FmapRet>) {
        return f(assert_value());
    } else {
        auto result = f(assert_value());
        if (result.is_error()) {
            return result.assert_error();
        }
        return result.assert_value();
    }
}

template<typename V>
template<typename Fmap>
auto ReturnValue<V>::fmap(Fmap&& f) && -> ReturnValue<decltype(strip_rv(f(std::declval<V&&>())))> {
    using FRetT = decltype(f(std::declval<V>()));
    using FmapRet = decltype(strip_rv(f(std::declval<V>())));
    if (is_error()) {
        return assert_error();
    }
    if constexpr (std::is_same_v<FRetT, FmapRet>) {
        return f(std::move(assert_value()));
    } else {
        auto result = f(std::move(assert_value()));
        if (result.is_error()) {
            return result.assert_error();
        }
        return result.assert_value();
    }
}

template<typename T, typename... Ts>
std::optional<std::error_code> any_is_error(const ReturnValue<T>& first, const ReturnValue<Ts>&... rest) {
    if (first.is_error()) {
        return first.assert_error();
    }
    if constexpr (sizeof...(rest) > 0) {
        return any_is_error(rest...);
    } else {
        return std::nullopt;
    }
}

template<typename Func, typename... Vs, typename T>
auto combine_with_values_cref_h(Func func, std::tuple<const Vs&...>&& values, const ReturnValue<T>& rv) {
    std::tuple<const T&> v{rv.assert_value()};
    auto final_tuple = std::tuple_cat(std::move(values), v);
    return std::apply(func, std::move(final_tuple));
}

template<typename F, typename... Vs, typename T, typename... Ts>
auto combine_with_values_cref_h(F func, std::tuple<const Vs&...>&& values, const ReturnValue<T>& rv, const ReturnValue<Ts>&... rest) {
    std::tuple<const T&> v{rv.assert_value()};
    auto next_tuple = std::tuple_cat(std::move(values), v);
    return combine_with_values_cref_h(func, std::move(next_tuple), std::forward<const ReturnValue<Ts>&>(rest)...);
}

// Base function to start the process with an empty tuple
template<typename Func, typename... Ts>
auto combine_with_values_cref(Func func, const ReturnValue<Ts>&... rvs) {
    return combine_with_values_cref_h(func, std::tuple<>{}, std::forward<const ReturnValue<Ts>&>(rvs)...);
}

template<typename F, typename... Ts>
auto combine(F&& f, const ReturnValue<Ts>&... rvs) -> ReturnValue<decltype(strip_rv(f(std::declval<const Ts&>()...)))> {
    if (auto maybe_error = any_is_error(rvs...); maybe_error.has_value()) {
        return maybe_error.value();
    }
    using FRetT = decltype(f(std::declval<const Ts&>()...));
    using FmapRet = decltype(strip_rv(f(std::declval<const Ts&>()...)));

    if constexpr (std::is_same_v<FRetT, FmapRet>) {
        return combine_with_values_cref(f, std::forward<const ReturnValue<Ts>&>(rvs)...);
    } else {
        auto result = combine_with_values_cref(f, std::forward<const ReturnValue<Ts>&>(rvs)...);
        if (result.is_error()) {
            return result.assert_error();
        }
        return result.assert_value();
    }
}

template<typename Func, typename... Vs, typename T>
auto combine_with_values_ref_h(Func func, std::tuple<Vs&...>&& values, ReturnValue<T>& rv) {
    std::tuple<T&> v{rv.assert_value()};
    auto final_tuple = std::tuple_cat(std::move(values), v);
    return std::apply(func, std::move(final_tuple));
}

template<typename F, typename... Vs, typename T, typename... Ts>
auto combine_with_values_ref_h(F func, std::tuple<Vs&...>&& values, ReturnValue<T>& rv, ReturnValue<Ts>&... rest) {
    std::tuple<T&> v{rv.assert_value()};
    auto next_tuple = std::tuple_cat(std::move(values), v);
    return combine_with_values_ref_h(func, std::move(next_tuple), std::forward<ReturnValue<Ts>&>(rest)...);
}

// Base function to start the process with an empty tuple
template<typename Func, typename... Ts>
auto combine_with_values_ref(Func func, ReturnValue<Ts>&... rvs) {
    return combine_with_values_ref_h(func, std::tuple<>{}, std::forward<ReturnValue<Ts>&>(rvs)...);
}

template<typename F, typename... Ts>
auto combine(F&& f, ReturnValue<Ts>&... rvs) -> ReturnValue<decltype(strip_rv(f(std::declval<Ts&>()...)))> {
    if (auto maybe_error = any_is_error(rvs...); maybe_error.has_value()) {
        return maybe_error.value();
    }
    using FRetT = decltype(f(std::declval<Ts&>()...));
    using FmapRet = decltype(strip_rv(f(std::declval<Ts&>()...)));

    if constexpr (std::is_same_v<FRetT, FmapRet>) {
        return combine_with_values_ref(f, std::forward<ReturnValue<Ts>&>(rvs)...);
    } else {
        auto result = combine_with_values(f, std::forward<ReturnValue<Ts>&>(rvs)...);
        if (result.is_error()) {
            return result.assert_error();
        }
        return result.assert_value();
    }
}

template<typename F, typename... Values, typename T>
auto combine_rv_with_values_h(F&& func, std::tuple<Values&&...>&& values, ReturnValue<T>&& rv) {
    std::tuple<T&&> v{std::move(rv.assert_value())};
    auto final_tuple = std::tuple_cat(std::move(values), std::move(v));
    return std::apply(func, std::move(final_tuple));
}

template<typename F, typename... Ts, typename T, typename... Rest>
auto combine_rv_with_values_h(F&& func, std::tuple<Ts&&...>&& values, ReturnValue<T>&& rv, ReturnValue<Rest>&&... rest) {
    std::tuple<T&&> v{std::move(rv.assert_value())};
    auto next_tuple = std::tuple_cat(std::move(values), std::move(v));
    return combine_rv_with_values_h(std::move(func), std::move(next_tuple), std::forward<ReturnValue<Rest>>(rest)...);
}

// Base function to start the process with an empty tuple
template<typename F, typename... Ts>
auto combine_rv_with_values(F&& func, ReturnValue<Ts>&&... rvs) {
    return combine_rv_with_values_h(std::move(func), std::tuple<>{}, std::forward<ReturnValue<Ts>>(rvs)...);
}

template<typename F, typename... Ts>
auto combine(F&& f, ReturnValue<Ts>&&... rvs) -> ReturnValue<decltype(strip_rv(f(std::declval<Ts&&>()...)))> {
    if (auto maybe_error = any_is_error(rvs...); maybe_error.has_value()) {
        return maybe_error.value();
    }
    using FRetT = decltype(f(std::declval<Ts&&>()...));
    using FmapRet = decltype(strip_rv(f(std::declval<Ts&&>()...)));

    if constexpr (std::is_same_v<FRetT, FmapRet>) {
        return combine_rv_with_values(std::move(f), std::forward<ReturnValue<Ts>>(rvs)...);
    } else {
        auto result = combine_rv_with_values(std::move(f), std::forward<ReturnValue<Ts>>(rvs)...);
        if (result.is_error()) {
            return result.assert_error();
        }
        return result.assert_value();
    }
}


}
