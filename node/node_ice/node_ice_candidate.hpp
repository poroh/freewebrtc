//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of ICE candidate functions to NAPI
//

#pragma once

#include <string_view>
#include <craftnapi/env.hpp>
#include "util/util_result.hpp"

namespace freewebrtc::node_ice {

Result<craftnapi::Value> ice_candidate_parse(craftnapi::Env& env, const craftnapi::CallbackInfo& ci);

}


