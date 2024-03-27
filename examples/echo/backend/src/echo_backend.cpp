#include <node_api.h>
#include "stun/stun_message.hpp"
#include "node/node_stun/node_stun_message.hpp"
#include "node/node_stun/node_stun_server_stateless.hpp"
#include "node/node_ice/node_ice_candidate.hpp"

namespace freewebrtc::napi {

napi_value init(napi_env inenv, napi_value) {
    Env env(inenv);
    auto exports =
        env.create_object({
            { "stun", env.create_object({
                     { "message_parse", env.create_function(node_stun::message_parse) },
                     { "StatelessServer", node_stun::server_stateless_class(env, "StatelessServer") }
                })
            },
            { "ice", env.create_object({
                    { "candidate_parse", env.create_function(node_ice::ice_candidate_parse) }
                })
            }
        });
    if (exports.is_err()) {
        env.throw_error("Failed to initialize: " + exports.unwrap_err().message());
        return nullptr;
    }
    return exports.unwrap().to_value().to_napi();
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init);

}  // namespace echo_backend
