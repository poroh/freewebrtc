// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP Client Auth bindings
//

#pragma once

#include "util/util_result.hpp"
#include "stun/stun_client_udp.hpp"
#include <craftnapi/env.hpp>

namespace freewebrtc::node_stun {

Result<stun::ClientUDP::Auth> parse_auth(craftnapi::Object obj);

}






