#include <node_api.h>
#include "stun/stun_message.hpp"

namespace echo_backend {

napi_value parse_stun(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    if (auto status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr); status != napi_ok) {
        return nullptr;
    }

    bool is_buffer;
    if (auto status = napi_is_buffer(env, args[0], &is_buffer); status != napi_ok) {
        return nullptr;
    }
    if (!is_buffer) {
        napi_throw_type_error(env, nullptr, "Argument should be a buffer");
        return nullptr;
    }

    void *data;
    size_t length;
    if (auto status = napi_get_buffer_info(env, args[0], &data, &length); status != napi_ok) {
        return nullptr;
    }
    freewebrtc::stun::ParseStat parsestat;
    auto maybe_msg = freewebrtc::stun::Message::parse(freewebrtc::util::ConstBinaryView(data, length), parsestat);
    if (!maybe_msg.has_value()) {
        napi_value err_obj;
        if (auto status = napi_create_object(env, &err_obj); status != napi_ok) {
            return nullptr;
        }
        napi_value error;
        if (auto status = napi_create_string_utf8(env, "Cannot parse", NAPI_AUTO_LENGTH, &error); status != napi_ok) {
            return nullptr;
        }

        if (auto status = napi_set_named_property(env, err_obj, "error", error); status != napi_ok) {
            return nullptr;
        }
        return err_obj;
    }

    napi_value obj;
    if (auto status = napi_create_object(env, &obj); status != napi_ok) {
        return nullptr;
    }

    const auto& v = *maybe_msg;
    napi_value cls;
    switch (v.header.cls.value()) {
    case freewebrtc::stun::Class::REQUEST:
        if (auto status = napi_create_string_utf8(env, "request", NAPI_AUTO_LENGTH, &cls); status != napi_ok) {
            return nullptr;
        }
        break;
    case freewebrtc::stun::Class::INDICATION:
        if (auto status = napi_create_string_utf8(env, "indication", NAPI_AUTO_LENGTH, &cls); status != napi_ok) {
            return nullptr;
        }
        break;
    case freewebrtc::stun::Class::SUCCESS_RESPONSE:
        if (auto status = napi_create_string_utf8(env, "success_response", NAPI_AUTO_LENGTH, &cls); status != napi_ok) {
            return nullptr;
        }
        break;
    case freewebrtc::stun::Class::ERROR_RESPONSE:
        if (auto status = napi_create_string_utf8(env, "error_response", NAPI_AUTO_LENGTH, &cls); status != napi_ok) {
            return nullptr;
        }
        break;
    }

    if (auto status = napi_set_named_property(env, obj, "class", cls); status != napi_ok) {
        return nullptr;
    }
    return obj;

}

napi_value init(napi_env env, napi_value exports) {
    napi_value fn;
    if (napi_status status = napi_create_function(env, nullptr, 0, parse_stun, nullptr, &fn); status != napi_ok) {
        return nullptr;
    }

    if (napi_status status = napi_set_named_property(env, exports, "parseSTUN", fn); status != napi_ok) {
        return nullptr;
    }

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init);

}  // namespace echo_backend
