//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// NAPI C++ wrapper
//

#include "napi_wrapper.hpp"
#include "napi_error.hpp"
#include "util/util_variant_overloaded.hpp"

namespace freewebrtc::napi {

Value::Value(napi_env env, napi_value value)
    : m_env(env)
    , m_value(value)
{}

ReturnValue<util::ConstBinaryView> Value::as_buffer() const noexcept {
    bool is_buffer;
    if (auto status = napi_is_buffer(m_env, m_value, &is_buffer); status != napi_ok) {
        return make_error_code(status);
    }
    if (!is_buffer) {
        return make_error_code(WrapperError::INVALID_TYPE);
    }

    void *data;
    size_t length;
    if (auto status = napi_get_buffer_info(m_env, m_value, &data, &length); status != napi_ok) {
        return make_error_code(status);
    }

    return freewebrtc::util::ConstBinaryView(data, length);
}


Object::Object(napi_env env, napi_value value)
    : m_env(env)
    , m_value(value)
{}

MaybeError Object::set_named_property(const std::string& k, const Value& v) noexcept {
    if (auto status = napi_set_named_property(m_env, m_value, k.c_str(), v.to_napi()); status != napi_ok) {
        return make_error_code(status);
    }
    return std::nullopt;
}

Env::Env(napi_env env)
    : m_env(env)
{}

ReturnValue<CallbackInfo> Env::create_callback_info(napi_callback_info info) const noexcept {
    size_t argc = 0;
    if (const auto status = napi_get_cb_info(m_env, info, &argc, nullptr, nullptr, nullptr); status != napi_ok) {
        return make_error_code(status);
    }

    std::vector<napi_value> args(argc);
    if (auto status = napi_get_cb_info(m_env, info, &argc, args.data(), nullptr, nullptr); status != napi_ok) {
        return make_error_code(status);
    }

    std::vector<Value> result_args;
    result_args.reserve(argc);
    for (const auto& v: args) {
        result_args.emplace_back(m_env, v);
    }
    return CallbackInfo{result_args};
}

ReturnValue<Object> Env::create_object() const noexcept {
    napi_value obj;
    if (auto status = napi_create_object(m_env, &obj); status != napi_ok) {
        return make_error_code(status);
    }
    return Object(m_env, obj);
}

ReturnValue<Object> Env::create_object(const ObjectSpec& spec) const noexcept {
    auto object_result = create_object();
    if (object_result.error().has_value()) {
        return object_result;
    }
    auto& object = object_result.value()->get();
    for (const auto& p: spec) {
        const auto& k = p.first;
        const auto& v = p.second;
        using RVV = ReturnValue<Value>;
        auto rvv = std::visit(
            util::overloaded {
                [](const ReturnValue<Value>& rv) -> RVV { return rv; },
                [](const Value& vv)  -> RVV { return vv; }
            }, v);
        if (rvv.error().has_value()) {
            return rvv.error().value();
        }
        const auto& vv = rvv.value()->get();
        if (auto maybe_error = object.set_named_property(k, vv); maybe_error.has_value()) {
            return maybe_error.value();
        }
    }
    return object_result;
}

ReturnValue<Value> Env::create_string(const std::string_view& str) const noexcept {
    napi_value result;
    if (auto status = napi_create_string_utf8(m_env, str.data(), str.size(), &result); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, result);
}

}
