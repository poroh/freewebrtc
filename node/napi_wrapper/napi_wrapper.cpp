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

ReturnValue<Object> Value::as_object() const noexcept {
    napi_valuetype valuetype;
    if (auto status = napi_typeof(m_env, m_value, &valuetype); status != napi_ok) {
        return make_error_code(status);
    }
    if (valuetype != napi_object) {
        return make_error_code(WrapperError::INVALID_TYPE);
    }
    return Object(m_env, m_value);
}

ReturnValue<std::string> Value::as_string() const noexcept {
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

ReturnValue<int32_t> Value::as_int32() const noexcept {
    int32_t result;
    if (auto status = napi_get_value_int32(m_env, m_value, &result); status != napi_ok) {
        return make_error_code(status);
    }
    return result;
}

ReturnValue<bool> Value::as_boolean() const noexcept {
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

ReturnValue<Value> Object::named_property(const std::string& k) const noexcept {
    napi_value result;
    if (auto status = napi_get_named_property(m_env, m_value, k.c_str(), &result); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, result);
}

ReturnValue<std::optional<Value>> Object::maybe_named_property(const std::string& k) const noexcept {
    bool has_property = false;
    if (auto status = napi_has_named_property(m_env, m_value, k.c_str(), &has_property); status != napi_ok) {
        return make_error_code(status);
    }
    if (!has_property) {
        return std::optional<Value>{std::nullopt};
    }
    return named_property(k)
        .fmap([](Value&& val) {
            return std::optional<Value>{std::move(val)};
        });
}


Env::Env(napi_env env)
    : m_env(env)
{}

ReturnValue<CallbackInfo> Env::create_callback_info(napi_callback_info info, void **data) const noexcept {
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

ReturnValue<Object> Env::create_object() const noexcept {
    napi_value obj;
    if (auto status = napi_create_object(m_env, &obj); status != napi_ok) {
        return make_error_code(status);
    }
    return Object(m_env, obj);
}

ReturnValue<Object> Env::create_object(const ObjectSpec& spec) const noexcept {
    auto object_result = create_object();
    if (object_result.is_error()) {
        return object_result;
    }
    auto& object = object_result.assert_value();
    for (const auto& p: spec) {
        const auto& k = p.first;
        const auto& v = p.second;
        using MaybeRVV = std::optional<ReturnValue<Value>>;
        using MaybeRVO = std::optional<ReturnValue<Object>>;
        auto mrvv = std::visit(
            util::overloaded {
                [](const ReturnValue<Value>& rv) -> MaybeRVV { return rv; },
                [](const Value& vv)  -> MaybeRVV { return vv; },
                [](const MaybeRVV& mrvv) { return mrvv; },
                [](const Object& vv)  -> MaybeRVV { return vv.to_value(); },
                [](const RVO& rv) -> MaybeRVV { return rv.fmap([](const auto& obj) { return obj.to_value(); }); },
                [](const MaybeRVO& mrvo) -> MaybeRVV {
                    if (!mrvo.has_value()) {
                        return std::nullopt;
                    }
                    return mrvo->fmap(Object::fmap_to_value);
                },
            }, v);
        if (!mrvv.has_value()) {
            continue;
        }
        const auto& rvv = mrvv.value();
        if (rvv.is_error()) {
            return rvv.assert_error();
        }
        const auto& vv = rvv.assert_value();
        if (auto maybe_error = object.set_named_property(k, vv); maybe_error.is_error()) {
            return maybe_error.assert_error();
        }
    }
    return object_result;
}

ReturnValue<Value> Env::create_undefined() const noexcept {
    napi_value result;
    if (auto status = napi_get_undefined(m_env, &result); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, result);
}

ReturnValue<Value> Env::create_string(const std::string_view& str) const noexcept {
    napi_value result;
    if (auto status = napi_create_string_utf8(m_env, str.data(), str.size(), &result); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, result);
}

ReturnValue<Value> Env::create_buffer(const util::ConstBinaryView& view) const noexcept {
    napi_value buffer;
    void *data;
    if (napi_status status = napi_create_buffer(m_env, view.size(), &data, &buffer); status != napi_ok) {
        return make_error_code(status);
    }
    memcpy(data, view.data(), view.size());
    return Value(m_env, buffer);
}

ReturnValue<Value> Env::create_boolean(bool value) const noexcept {
    napi_value boolean;
    if (napi_status status = napi_get_boolean(m_env, value, &boolean); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, boolean);
}

ReturnValue<Value> Env::create_int32(int32_t value) const noexcept {
    napi_value int32;
    if (napi_status status = napi_create_int32(m_env, value, &int32); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, int32);
}

ReturnValue<Value> Env::create_uint32(int32_t value) const noexcept {
    napi_value uint32;
    if (napi_status status = napi_create_uint32(m_env, value, &uint32); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, uint32);
}

ReturnValue<Value> Env::create_bigint_uint64(uint64_t value) const noexcept {
    napi_value result;
    if (napi_status status = napi_create_bigint_uint64(m_env, value, &result); status != napi_ok) {
        return make_error_code(status);
    }
    return Value(m_env, result);
}

ReturnValue<Value> Env::throw_error(const std::string& message) const noexcept {
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
    auto ret_rvv = f(env, ci_info_rvv.assert_value());
    if (env.maybe_throw_error(ret_rvv)) {
        return nullptr;
    }
    return ret_rvv.assert_value().to_napi();
}

ReturnValue<Value> Env::create_function(Function f, std::optional<std::string_view> maybename) {
    napi_value result;
    auto data = std::make_unique<Function>(std::move(f));
    if (maybename.has_value()) {
        const auto& name = maybename.value();
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
    if (!attr.getter.has_value()) {
        return nullptr;
    }
    auto ret_rvv = (*attr.getter)(env, ci_info_rvv.assert_value());
    if (env.maybe_throw_error(ret_rvv)) {
        return nullptr;
    }
    return ret_rvv.assert_value().to_napi();
}

static napi_value function_setter(napi_env inenv, napi_callback_info info) {
    Env env(inenv);
    void *data;
    auto ci_info_rvv = env.create_callback_info(info, &data);
    if (env.maybe_throw_error(ci_info_rvv)) {
        return nullptr;
    }
    auto& attr = *static_cast<Env::ClassAttr *>(data);
    if (!attr.setter.has_value()) {
        return nullptr;
    }
    auto ret_rvv = (*attr.setter)(env, ci_info_rvv.assert_value());
    if (env.maybe_throw_error(ret_rvv)) {
        return nullptr;
    }
    return ret_rvv.assert_value().to_napi();
}

ReturnValue<Value> Env::create_class(std::string_view name, Function ctor, const ClassPropertySpec& spec) const noexcept {
    using DescrRV = ReturnValue<napi_property_descriptor>;
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
                    [&](const ReturnValue<Value>& rvv) -> DescrRV {
                            if (rvv.is_error()) {
                                return rvv.assert_error();
                            }
                            return napi_property_descriptor{
                                p.first.c_str(),
                                nullptr, // name
                                function_wrapper, // method
                                nullptr, // getter
                                nullptr, // setter
                                rvv.assert_value().to_napi(), // value
                                napi_default, // attributes
                                nullptr  // data
                            };
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
                p.second);
        if (rvv.is_error()) {
            return rvv.assert_error();
        }
        descriptors.emplace_back(rvv.assert_value());
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

ReturnValue<Value> CallbackInfo::operator[](size_t index) const noexcept {
    if (index >= m_args.size()) {
        return make_error_code(WrapperError::NO_REQUIRED_ARGUMENT);
    }
    return m_args[index];
}

}
