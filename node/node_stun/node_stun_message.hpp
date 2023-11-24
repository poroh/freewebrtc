//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN to node.js type
//

#pragma once

#include "stun/stun_message.hpp"
#include "node/napi_wrapper/napi_wrapper.hpp"

namespace freewebrtc::node_stun {

ReturnValue<napi::Object> message(const napi::Env& env, const stun::Message& msg);
ReturnValue<napi::Value> message_parse(napi::Env& env, const napi::CallbackInfo& ci);

}
