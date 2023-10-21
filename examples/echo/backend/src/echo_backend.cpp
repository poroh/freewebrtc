#include <node_api.h>
#include "stun/stun_message.hpp"
#include "napi_stun_message.hpp"

namespace freewebrtc::napi {

napi_value parse_stun(napi_env inenv, napi_callback_info info) {

    Env env(inenv);

    auto ci_result = env.create_callback_info(info);
    if (ci_result.error().has_value()) {
        return nullptr;
    }

    const auto& ci = ci_result.value().value().get();
    if (ci.args.size() == 0) {
        return env.throw_error("First argument must be a buffer");
    }

    auto as_buffer_result = ci.args[0].as_buffer();
    if (env.maybe_throw_error(as_buffer_result)) {
        return nullptr;
    }

    const auto& view = as_buffer_result.value()->get();

    freewebrtc::stun::ParseStat parsestat;
    auto maybe_msg = freewebrtc::stun::Message::parse(view, parsestat);
    if (!maybe_msg.has_value()) {
        return env.throw_error("Failed to parse STUN packet");
    }
    const auto& msg = maybe_msg.value();

    auto napi_msg_result = stun_message(env, msg);
    if (env.maybe_throw_error(napi_msg_result)) {
        return nullptr;
    }
    return napi_msg_result.value()->get().to_napi();

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
