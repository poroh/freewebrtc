// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN UDP Client Auth bindings
//

#pragma once

#include "util/util_result.hpp"
#include "stun/stun_client_udp.hpp"
#include "node/napi_wrapper/napi_wrapper.hpp"

namespace freewebrtc::node_stun {

Result<stun::ClientUDP::Auth> parse_auth(napi::Object obj);

}






