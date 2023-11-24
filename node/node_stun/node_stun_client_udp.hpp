//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP client to NAPI
//

#pragma once

#include <string_view>
#include "node/napi_wrapper/napi_wrapper.hpp"

namespace freewebrtc::node_stun {

ReturnValue<napi::Value> client_udp_class(napi::Env& env, std::string_view name);

}
