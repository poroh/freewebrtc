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
#include <functional>
#include <memory>
#include "util/util_result.hpp"
#include "util/util_binary_view.hpp"
#include "napi_error.hpp"

namespace freewebrtc::napi {

class Buffer;
class Value;

class Object {
public:
    Object(napi_env, napi_value);

    MaybeError set_named_property(const std::string&, const Value&) noexcept;
    Value to_value() const noexcept;
    Result<Value> named_property(const std::string& name) const noexcept;
    Result<std::optional<Value>> maybe_named_property(const std::string& name) const noexcept;

    template<typename T>
    Result<Value> wrap(std::unique_ptr<T> native_obj) const noexcept;

    static Value fmap_to_value(const Object&) noexcept;
private:
    napi_env m_env;
    napi_value m_value;
};

class Array {
public:
    Array(napi_env, napi_value);

    Value to_value() const noexcept;
    MaybeError set_element(size_t i, const Value&) noexcept;

    static Value fmap_to_value(const Array&) noexcept;
private:
    napi_env m_env;
    napi_value m_value;
};

class Value {
public:
    Value(napi_env, napi_value);

    Result<util::ConstBinaryView> as_buffer() const noexcept;
    Result<Object> as_object() const noexcept;
    Result<std::string> as_string() const noexcept;
    Result<int32_t> as_int32() const noexcept;
    Result<bool> as_boolean() const noexcept;

    napi_value to_napi() const noexcept;

    template<typename T>
    Result<std::reference_wrapper<T>> unwrap() const noexcept;

    static Result<util::ConstBinaryView> to_buffer(const Value& obj) noexcept;
    static Result<Object> to_object(const Value& obj) noexcept;
    static Result<std::string> to_string(const Value& obj) noexcept;
    static Result<int32_t> to_int32(const Value& obj) noexcept;

private:
    napi_env m_env;
    napi_value m_value;
};

class CallbackInfo {
public:
    CallbackInfo(Value this_arg, std::vector<Value>&&);

    Value this_arg;

    Result<Value> operator[](size_t index) const noexcept;

private:
    std::vector<Value> m_args;
};

class Env {
public:
    explicit Env(napi_env);

    Result<CallbackInfo> create_callback_info(napi_callback_info, void ** = nullptr) const noexcept;

    using RVV = Result<Value>;
    using RVO = Result<Object>;
    using RVA = Result<Array>;

    using ValueInit = std::variant<Value, RVV, std::optional<RVV>, Object, RVO, std::optional<RVO>>;
    using ObjectSpec = std::vector<std::pair<std::string, ValueInit>>;
    using Function = std::function<RVV(Env& env, const CallbackInfo&)>;

    RVO create_object() const noexcept;
    RVO create_object(const ObjectSpec&) const noexcept;

    RVA create_array() const noexcept;

    template<typename Container, typename F>
    RVV create_array(const Container&, F&&);

    RVV create_undefined() const noexcept;
    RVV create_null() const noexcept;
    RVV create_string(const std::string_view&) const noexcept;
    RVV create_buffer(const util::ConstBinaryView& view) const noexcept;
    RVV create_boolean(bool value) const noexcept;
    RVV create_int32(int32_t value) const noexcept;
    RVV create_uint32(int32_t value) const noexcept;
    RVV create_bigint_uint64(uint64_t value) const noexcept;
    RVV create_function(Function, std::optional<std::string_view> name = std::nullopt);
    RVV throw_error(const std::string& message) const noexcept;

    struct ClassAttr {
        std::optional<Function> getter;
        std::optional<Function> setter;
    };
    using ClassProperty = std::variant<Function, RVV, ClassAttr>;
    using ClassPropertySpec = std::vector<std::pair<std::string, ClassProperty>>;
    RVV create_class(std::string_view name, Function ctor, const ClassPropertySpec&) const noexcept;

    template<typename T>
    bool maybe_throw_error(const Result<T>& v) const noexcept;
private:
    napi_env m_env;
};

//
// implementation
//
template<typename T>
Result<Value> Object::wrap(std::unique_ptr<T> native_obj) const noexcept {
    auto dtor = [](napi_env, void *data, void *) {
        delete static_cast<T *>(data);
    };
    if (auto status = napi_wrap(m_env, m_value, native_obj.get(), dtor, nullptr, nullptr); status != napi_ok) {
        return make_error_code(status);
    }
    native_obj.release();
    return to_value();
}

inline napi_value Value::to_napi() const noexcept {
    return m_value;
}

template<typename T>
Result<std::reference_wrapper<T>> Value::unwrap() const noexcept {
    T *t;
    if (auto status = napi_unwrap(m_env, m_value, reinterpret_cast<void **>(&t)); status != napi_ok) {
        return make_error_code(status);
    }
    return std::ref(*t);
}

template<typename T>
bool Env::maybe_throw_error(const Result<T>& v) const noexcept {
    if (v.is_err()) {
        throw_error(v.unwrap_err().message());
        return true;
    }
    return false;
}

inline Value Object::to_value() const noexcept {
    return Value(m_env, m_value);
}

inline Value Object::fmap_to_value(const Object& obj) noexcept {
    return obj.to_value();
}

inline Value Array::to_value() const noexcept {
    return Value(m_env, m_value);
}

inline Value Array::fmap_to_value(const Array& arr) noexcept {
    return arr.to_value();
}

inline Result<util::ConstBinaryView> Value::to_buffer(const Value& val) noexcept {
    return val.as_buffer();
}

inline Result<Object> Value::to_object(const Value& val) noexcept {
    return val.as_object();
}

inline Result<std::string> Value::to_string(const Value& val) noexcept {
    return val.as_string();
}

inline Result<int32_t> Value::to_int32(const Value& val) noexcept {
    return val.as_int32();
}

template<typename Container, typename F>
Env::RVV Env::create_array(const Container& c, F&& f) {
    return create_array()
        > [&](auto&& array) -> RVA {
            size_t i = 0;
            for (const auto& v: c) {
                auto maybe_err = f(v)
                    .bind([&](auto&& napiv) {
                        return array.set_element(i++, napiv);
                    });
                if (maybe_err.is_err()) {
                    return maybe_err.unwrap_err();
                }
            }
            return array;
        }
        > Array::fmap_to_value;
}


}
