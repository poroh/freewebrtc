//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// NAPI C++ wrapper
//

#pragma once

#include <node_api.h>
#include <vector>
#include <optional>
#include "util/util_return_value.hpp"
#include "util/util_binary_view.hpp"

namespace freewebrtc::napi {

class Buffer;
class Value;

using MaybeError = std::optional<std::error_code>;

class Object {
public:
    Object(napi_env, napi_value);

    MaybeError set_named_property(const std::string&, const Value&) noexcept;
    Value to_value() const noexcept;

private:
    napi_env m_env;
    napi_value m_value;
};

class Value {
public:
    Value(napi_env, napi_value);

    ReturnValue<util::ConstBinaryView> as_buffer() const noexcept;


    napi_value to_napi() const noexcept;

private:
    napi_env m_env;
    napi_value m_value;
};

struct CallbackInfo {
    std::vector<Value> args;
};

class Env {
public:
    explicit Env(napi_env);

    ReturnValue<CallbackInfo> create_callback_info(napi_callback_info) const noexcept;

    using ValueInit = std::variant<Value, ReturnValue<Value>>;
    using ObjectSpec = std::vector<std::pair<std::string, ValueInit>>;

    ReturnValue<Object> create_object() const noexcept;
    ReturnValue<Object> create_object(const ObjectSpec&) const noexcept;

    ReturnValue<Value> create_string(const std::string_view&) const noexcept;

    napi_value throw_error(const std::string& message) const noexcept;

    template<typename T>
    bool maybe_throw_error(const ReturnValue<T>& v) const noexcept;
private:
    napi_env m_env;
};

//
// implementation
//
inline napi_value Value::to_napi() const noexcept {
    return m_value;
}

inline napi_value Env::throw_error(const std::string& message) const noexcept {
    napi_throw_error(m_env, nullptr, message.c_str());
    return nullptr;
}

template<typename T>
bool Env::maybe_throw_error(const ReturnValue<T>& v) const noexcept {
    if (v.error().has_value()) {
        throw_error(v.error()->message());
        return true;
    }
    return false;
}

inline Value Object::to_value() const noexcept {
    return Value(m_env, m_value);
}

}



