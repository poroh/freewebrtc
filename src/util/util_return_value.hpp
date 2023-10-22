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
#include <optional>

namespace freewebrtc {

template<typename V>
class ReturnValue {
public:
    using Value = V;
    using Error = std::error_code;

    ReturnValue(Value&&);
    ReturnValue(const Value&);
    ReturnValue(std::error_code&&);
    ReturnValue(const std::error_code&);
    ReturnValue(const ReturnValue&) = default;
    ReturnValue(ReturnValue&&) = default;

    using MaybeError = std::optional<Error>;
    MaybeError error() const noexcept;
    using MaybeValue = std::optional<std::reference_wrapper<Value>>;
    using MaybeConstValue = std::optional<std::reference_wrapper<const Value>>;
    MaybeConstValue value() const noexcept;
    MaybeValue value() noexcept;

    // Apply function to Value if it ReturnValue is value
    // and return result as ReturnValue of result of the function.
    // If Fmap returns ReturnValue<SomeType> then return value of fmap is
    // ReturnValue<SomeType> instead of ResultValue<ResultValue<SomeType>>
    template<typename Fmap>
    auto fmap(Fmap&& f) -> ReturnValue<decltype(f(std::declval<V>()))>;

private:
    template<typename T>
    ReturnValue<T> fmap_helper(ReturnValue<T>&& v);
    template<typename T>
    ReturnValue<T> fmap_helper(T&& v);

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
inline std::optional<typename ReturnValue<V>::Error>
ReturnValue<V>::error() const noexcept {
    auto err = std::get_if<Error>(&m_result);
    return err != nullptr ? *err : std::optional<Error>(std::nullopt);
}

template<typename V>
inline typename ReturnValue<V>::MaybeConstValue
ReturnValue<V>::value() const noexcept {
    auto v = std::get_if<V>(&m_result);
    return v != nullptr ? MaybeConstValue{*v} : std::nullopt;
}

template<typename V>
inline typename ReturnValue<V>::MaybeValue
ReturnValue<V>::value() noexcept {
    auto v = std::get_if<V>(&m_result);
    return v != nullptr ? MaybeValue{*v} : std::nullopt;
}

template<typename V>
template<typename T>
ReturnValue<T> ReturnValue<V>::fmap_helper(ReturnValue<T>&& v) {
    if (v.error().has_value()) {
        return v.error().value();
    }
    return std::move(v.value().value().get());
};

template<typename V>
template<typename T>
ReturnValue<T> ReturnValue<V>::fmap_helper(T&& v) {
    return v;
};

template<typename T>
template<typename Fmap>
auto ReturnValue<T>::fmap(Fmap&& f) -> ReturnValue<decltype(f(std::declval<T>()))> {
    if (error().has_value()) {
        return error().value();
    }
    return fmap_helper(f(value()->get()));
}

}
