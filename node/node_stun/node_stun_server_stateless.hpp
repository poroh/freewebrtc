//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of stateless STUN server to NAPI
//

#pragma once

#include <string_view>
#include "node/napi_wrapper/napi_wrapper.hpp"

namespace freewebrtc::node_stun {

Result<napi::Value> server_stateless_class(napi::Env& env, std::string_view name);

}
