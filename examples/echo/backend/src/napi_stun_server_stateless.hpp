//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of stateless STUN server to NAPI
//

#pragma once

#include <string_view>
#include "napi_wrapper.hpp"

namespace freewebrtc::napi {

ReturnValue<Value> stun_server_class(Env& env, std::string_view name);

}
