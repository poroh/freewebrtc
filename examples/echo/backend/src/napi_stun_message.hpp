//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN to node.js type
//

#pragma once

#include "stun/stun_message.hpp"
#include "napi_wrapper.hpp"

namespace freewebrtc::napi {

ReturnValue<Value> stun_message(const Env& env, const stun::Message& msg);

}
