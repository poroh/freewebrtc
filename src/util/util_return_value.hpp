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
    ReturnValue(std::error_code&&);
    ReturnValue(const std::error_code&);
    ReturnValue(const ReturnValue&) = default;
    ReturnValue(ReturnValue&&) = default;

    std::optional<Error> error() const noexcept;
    const Value *value() const noexcept;
    Value *value() noexcept;

private:
    std::variant<Value, Error> m_result;
};

//
// inlines
//
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
inline const V *ReturnValue<V>::value() const noexcept {
    return std::get_if<V>(&m_result);
}

template<typename V>
inline V *ReturnValue<V>::value() noexcept {
    return std::get_if<V>(&m_result);
}

}
