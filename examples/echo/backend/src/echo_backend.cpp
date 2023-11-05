#include <node_api.h>
#include "stun/stun_message.hpp"
#include "napi_stun_message.hpp"
#include "napi_stun_server_stateless.hpp"

namespace freewebrtc::napi {

napi_value init(napi_env inenv, napi_value) {
    Env env(inenv);
    auto exports =
        env.create_object({
                {"stun", env.create_object({
                            { "message_parse", env.create_function(stun_message_parse) },
                            { "StatelessServer", stun_server_class(env, "StatelessServer") }
                        })
                }
            });
    if (exports.is_error()) {
        env.throw_error("Failed to initialize: " + exports.assert_error().message());
        return nullptr;
    }
    return exports.assert_value().to_value().to_napi();
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init);

}  // namespace echo_backend
