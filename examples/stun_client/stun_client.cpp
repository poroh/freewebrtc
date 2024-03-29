#include <node_api.h>
#include "stun/stun_message.hpp"
#include "node/node_stun/node_stun_message.hpp"
#include "node/node_stun/node_stun_client_udp.hpp"

namespace freewebrtc::napi {

napi_value init(napi_env inenv, napi_value) {
    Env env(inenv);
    auto exports =
        env.create_object({
                { "message_parse", env.create_function(node_stun::message_parse) },
                { "ClientUDP", node_stun::client_udp_class(env, "ClientUDP") }
            });
    if (exports.is_error()) {
        env.throw_error("Failed to initialize: " + exports.assert_error().message());
        return nullptr;
    }
    return exports.assert_value().to_value().to_napi();
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init);

}  // namespace echo_backend
