//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP client effects to NAPI
//

#pragma once

#include "stun/stun_client_udp.hpp"
#include "node/napi_wrapper/napi_wrapper.hpp"

namespace freewebrtc::node_stun {

Result<napi::Object> client_udp_effect_to_napi(napi::Env&, const stun::ClientUDP::Effect& effect);

}
