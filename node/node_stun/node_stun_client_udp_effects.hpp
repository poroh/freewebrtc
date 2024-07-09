//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP client effects to NAPI
//

#pragma once

#include "stun/stun_client_udp.hpp"
#include <craftnapi/env.hpp>

namespace freewebrtc::node_stun {

Result<craftnapi::Object> client_udp_effect_to_craftnapi(craftnapi::Env&, const stun::ClientUDP::Effect& effect);

}
