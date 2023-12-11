//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of ICE candidate functions to NAPI
//

#pragma once

#include <string_view>
#include "node/napi_wrapper/napi_wrapper.hpp"

namespace freewebrtc::node_ice {

ReturnValue<napi::Value> ice_candidate_parse(napi::Env& env, const napi::CallbackInfo& ci);

}


