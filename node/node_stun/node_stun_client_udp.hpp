//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP client to NAPI
//

#pragma once

#include <string_view>
#include <craftnapi/env.hpp>

namespace freewebrtc::node_stun {

Result<craftnapi::Value> client_udp_class(craftnapi::Env& env, std::string_view name);

}
