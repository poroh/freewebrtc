//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP client settings to NAPI
//

#pragma once

#include "stun/stun_client_udp_settings.hpp"
#include <craftnapi/env.hpp>

namespace freewebrtc::node_stun {

Result<stun::client_udp::Settings> client_udp_settings_from_craftnapi(craftnapi::Object obj);

}



