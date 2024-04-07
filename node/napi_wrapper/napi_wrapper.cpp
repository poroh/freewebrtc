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

Result<util::ConstBinaryView> Value::as_buffer() const noexcept {
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

Result<Object> Value::as_object() const noexcept {
    napi_valuetype valuetype;
    if (auto status = napi_typeof(m_env, m_value, &valuetype); status != napi_ok) {
        return make_error_code(status);
    }
    if (valuetype != napi_object) {
        return make_error_code(WrapperError::INVALID_TYPE);
    }
    return Object(m_env, m_value);
}

Result<std::string> Value::as_string() const noexcept {
    // Get the length of the string in bytes
    size_t sz;
    if (auto status = napi_get_value_string_utf8(m_env, m_value, nullptr, 0, &sz); status != napi_ok) {
        return make_error_code(status);
    }

    std::string str;
    str.resize(sz);

    if (auto status = napi_get_value_string_utf8(m_env, m_value, &str[0], sz + 1, &sz); status != napi_ok) {
        return make_error_code(status);
    }

    return str;
}

Result<int32_t> Value::as_int32() const noexcept {
    int32_t result;
    if (auto status = napi_get_value_int32(m_env, m_value, &result); status != napi_ok) {
        return make_error_code(status);
    }
    return result;
}

Result<bool> Value::as_boolean() const noexcept {
    bool result;
    if (auto status = napi_get_value_bool(m_env, m_value, &result); status != napi_ok) {
        return make_error_code(status);
    }
    return result;
}

Object::Object(napi_env env, napi_value value)
    : m_env(env)
    , m_value(value)
{}

MaybeError Object::set_named_property(const std::string& k, const Value& v) noexcept {
    if (auto status = napi_set_named_property(m_env, m_value, k.c_str(), v.to_napi()); status != napi_ok) {
        return make_error_code(status);
    }
    return success();
}

Result<Value> Object::named_property(const std::string& k) const noexcept {
    napi_value result;
    if (auto status = napi_get_named_property(m_env, m_value, k.c_str(), &result); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, result);
}

Result<Maybe<Value>> Object::maybe_named_property(const std::string& k) const noexcept {
    bool has_property = false;
    if (auto status = napi_has_named_property(m_env, m_value, k.c_str(), &has_property); status != napi_ok) {
        return make_error_code(status);
    }
    if (!has_property) {
        return Maybe<Value>{none()};
    }
    return named_property(k)
        .fmap([](Value&& val) {
            return Maybe<Value>{std::move(val)};
        });
}

Array::Array(napi_env env, napi_value v)
    : m_env(env)
    , m_value(v)
{}

MaybeError Array::set_element(size_t i, const Value& v) noexcept {
    if (auto status = napi_set_element(m_env, m_value, i, v.to_napi())) {
        return make_error_code(status);
    }
    return success();
}

Env::Env(napi_env env)
    : m_env(env)
{}

Result<CallbackInfo> Env::create_callback_info(napi_callback_info info, void **data) const noexcept {
    size_t argc = 0;
    napi_value this_arg;
    if (const auto status = napi_get_cb_info(m_env, info, &argc, nullptr, &this_arg, data); status != napi_ok) {
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
    return CallbackInfo(Value{m_env, this_arg}, std::move(result_args));
}

Result<Object> Env::create_object() const noexcept {
    napi_value obj;
    if (auto status = napi_create_object(m_env, &obj); status != napi_ok) {
        return make_error_code(status);
    }
    return Object(m_env, obj);
}

Result<Object> Env::create_object(const ObjectSpec& spec) const noexcept {
    auto object_result = create_object();
    if (object_result.is_err()) {
        return object_result;
    }
    auto& object = object_result.unwrap();
    for (const auto& p: spec) {
        const auto& k = p.first;
        const auto& v = p.second;
        using MaybeRVV = Maybe<Result<Value>>;
        using MaybeRVO = Maybe<Result<Object>>;
        auto mrvv = std::visit(
            util::overloaded {
                [](const Result<Value>& rv) -> MaybeRVV { return rv; },
                [](const Value& vv) -> MaybeRVV { return Result<Value>{vv}; },
                [](const MaybeRVV& mrvv) { return mrvv; },
                [](const Object& vv)  -> MaybeRVV { return Result<Value>{vv.to_value()}; },
                [](const RVO& rv) -> MaybeRVV { return rv.fmap([](const auto& obj) { return obj.to_value(); }); },
                [](const MaybeRVO& mrvo) -> MaybeRVV {
                    return mrvo.fmap([](auto&& rv) {
                        return rv.fmap(Object::fmap_to_value);
                    });
                }
            }, v);
        if (!mrvv.is_some()) {
            continue;
        }
        auto maybe_err = mrvv.unwrap()
            .bind([&](auto&& vv) { return object.set_named_property(k, vv); });
        if (maybe_err.is_err()) {
            maybe_err.add_context(k + " attribute");
            return maybe_err.unwrap_err();
        }
    }
    return object_result;
}

Result<Array> Env::create_array() const noexcept {
    napi_value arr;
    if (auto status = napi_create_array(m_env, &arr); status != napi_ok) {
        return make_error_code(status);
    }
    return Array(m_env, arr);
}

Result<Value> Env::create_undefined() const noexcept {
    napi_value result;
    if (auto status = napi_get_undefined(m_env, &result); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, result);
}

Result<Value> Env::create_null() const noexcept {
    napi_value result;
    if (auto status = napi_get_null(m_env, &result); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, result);
}

Result<Value> Env::create_string(const std::string_view& str) const noexcept {
    napi_value result;
    if (auto status = napi_create_string_utf8(m_env, str.data(), str.size(), &result); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, result);
}

Result<Value> Env::create_buffer(const util::ConstBinaryView& view) const noexcept {
    napi_value buffer;
    void *data;
    if (napi_status status = napi_create_buffer(m_env, view.size(), &data, &buffer); status != napi_ok) {
        return make_error_code(status);
    }
    memcpy(data, view.data(), view.size());
    return Value(m_env, buffer);
}

Result<Value> Env::create_boolean(bool value) const noexcept {
    napi_value boolean;
    if (napi_status status = napi_get_boolean(m_env, value, &boolean); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, boolean);
}

Result<Value> Env::create_int32(int32_t value) const noexcept {
    napi_value int32;
    if (napi_status status = napi_create_int32(m_env, value, &int32); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, int32);
}

Result<Value> Env::create_uint32(int32_t value) const noexcept {
    napi_value uint32;
    if (napi_status status = napi_create_uint32(m_env, value, &uint32); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, uint32);
}

Result<Value> Env::create_bigint_uint64(uint64_t value) const noexcept {
    napi_value result;
    if (napi_status status = napi_create_bigint_uint64(m_env, value, &result); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, result);
}

Result<Value> Env::throw_error(const std::string& message) const noexcept {
    if (napi_status status = napi_throw_error(m_env, nullptr, message.c_str()); status != napi_ok) {
        return make_error_code(status);
    }
    return create_undefined();
}

static napi_value function_wrapper(napi_env inenv, napi_callback_info info) {
    Env env(inenv);
    void *data;
    auto ci_info_rvv = env.create_callback_info(info, &data);
    if (env.maybe_throw_error(ci_info_rvv)) {
        return nullptr;
    }
    auto& f = *static_cast<Env::Function *>(data);
    auto ret_rvv = f(env, ci_info_rvv.unwrap());
    if (env.maybe_throw_error(ret_rvv)) {
        return nullptr;
    }
    return ret_rvv.unwrap().to_napi();
}

Result<Value> Env::create_function(Function f, Maybe<std::string_view> maybename) {
    napi_value result;
    auto data = std::make_unique<Function>(std::move(f));
    if (maybename.is_some()) {
        const auto& name = maybename.unwrap();
        if (auto status = napi_create_function(m_env, name.data(), name.size(), function_wrapper, data.get(), &result); status != napi_ok) {
            return make_error_code(status);
        }
    } else {
        if (auto status = napi_create_function(m_env, nullptr, 0, function_wrapper, data.get(), &result); status != napi_ok) {
            return make_error_code(status);
        }
    }
    auto dtor = [](napi_env, void *data, void *) {
        delete static_cast<Function *>(data);
    };
    if (auto status = napi_add_finalizer(m_env, result, data.get(), dtor, nullptr, nullptr); status != napi_ok) {
        return make_error_code(status);
    }
    data.release();
    return Value(m_env, result);
}

static napi_value function_getter(napi_env inenv, napi_callback_info info) {
    Env env(inenv);
    void *data;
    auto ci_info_rvv = env.create_callback_info(info, &data);
    if (env.maybe_throw_error(ci_info_rvv)) {
        return nullptr;
    }
    auto& attr = *static_cast<Env::ClassAttr *>(data);
    if (attr.getter.is_none()) {
        return nullptr;
    }
    auto ret_rvv = attr.getter.unwrap()(env, ci_info_rvv.unwrap());
    if (env.maybe_throw_error(ret_rvv)) {
        return nullptr;
    }
    return ret_rvv.unwrap().to_napi();
}

static napi_value function_setter(napi_env inenv, napi_callback_info info) {
    Env env(inenv);
    void *data;
    auto ci_info_rvv = env.create_callback_info(info, &data);
    if (env.maybe_throw_error(ci_info_rvv)) {
        return nullptr;
    }
    auto& attr = *static_cast<Env::ClassAttr *>(data);
    if (attr.setter.is_none()) {
        return nullptr;
    }
    auto ret_rvv = attr.setter.unwrap()(env, ci_info_rvv.unwrap());
    if (env.maybe_throw_error(ret_rvv)) {
        return nullptr;
    }
    return ret_rvv.unwrap().to_napi();
}

Result<Value> Env::create_class(std::string_view name, Function ctor, const ClassPropertySpec& spec) const noexcept {
    using DescrRV = Result<napi_property_descriptor>;
    std::vector<napi_property_descriptor> descriptors;
    descriptors.reserve(spec.size());
    for (auto& p: spec) {
        auto rvv =
            std::visit(
                util::overloaded {
                    [&](const Function& f) -> DescrRV {
                        return napi_property_descriptor{
                                p.first.c_str(),
                                nullptr, // name
                                function_wrapper, // method
                                nullptr, // getter
                                nullptr, // setter
                                nullptr, // value
                                napi_default, // attributes
                                new Function(f) // data
                            };
                    },
                    [&](const Result<Value>& rvv) -> DescrRV {
                        return rvv.fmap([&](auto&& v) {
                            return napi_property_descriptor{
                                p.first.c_str(),
                                nullptr, // name
                                function_wrapper, // method
                                nullptr, // getter
                                nullptr, // setter
                                v.to_napi(), // value
                                napi_default, // attributes
                                nullptr  // data
                            };
                        });
                    },
                    [&](const ClassAttr& attr) -> DescrRV {
                        return napi_property_descriptor{
                            p.first.c_str(),
                            nullptr, // name
                            nullptr, // method
                            function_getter, // getter
                            function_setter, // setter
                            nullptr, // value
                            napi_default, // attributes
                            new ClassAttr(attr)  // data
                        };
                    }
                },
                p.second)
            .add_context("attribute create", p.first)
            .fmap([&](auto&& v) {
                descriptors.emplace_back(std::move(v));
                return Unit{};
            });
        if (rvv.is_err()) {
            return rvv.unwrap_err();
        }
    }
    napi_value result;
    auto status =
        napi_define_class(
            m_env,
            name.data(),
            name.size(),
            function_wrapper,
            new Function(ctor),
            descriptors.size(),
            descriptors.data(),
            &result);
    if (status != napi_ok) {
        return make_error_code(status);
    }
    // Note that data in property descriptions data will leak
    // if module can be uploaded. We can add napi_add_finalizer to
    // class definition as well but it is not practal thing.
    return Value(m_env, result);
}

CallbackInfo::CallbackInfo(Value self_arg, std::vector<Value>&& values)
    : this_arg(self_arg)
    , m_args(std::move(values))
{}

Result<Value> CallbackInfo::operator[](size_t index) const noexcept {
    if (index >= m_args.size()) {
        return make_error_code(WrapperError::NO_REQUIRED_ARGUMENT);
    }
    return m_args[index];
}

}
