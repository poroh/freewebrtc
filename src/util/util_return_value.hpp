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

namespace freewebrtc {

template<typename V>
class ReturnValue {
    template<typename T> T strip_rv(const ReturnValue<T>& v);
    template<typename T> T strip_rv(const T& v);

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
    auto fmap(Fmap&& f) -> ReturnValue<decltype(strip_rv(f(std::declval<V>())))>;

private:
    std::variant<Value, Error> m_result;
};

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
auto ReturnValue<V>::fmap(Fmap&& f) -> ReturnValue<decltype(strip_rv(f(std::declval<V>())))> {
    using FRetT = decltype(f(std::declval<V>()));
    using FmapRet = decltype(strip_rv(f(std::declval<V>())));
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

}
