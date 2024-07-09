//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN to node.js type
//

#pragma once

#include <craftnapi/env.hpp>

#include "stun/stun_message.hpp"
#include "util/util_result.hpp"

namespace freewebrtc::node_stun {

Result<craftnapi::Object> message(const craftnapi::Env& env, const stun::Message& msg);
Result<craftnapi::Value> message_parse(craftnapi::Env& env, const craftnapi::CallbackInfo& ci);

}
