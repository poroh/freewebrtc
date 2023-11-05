//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN to node.js type
//

#pragma once

#include "stun/stun_header.hpp"
#include "napi_wrapper.hpp"

namespace freewebrtc::napi {

ReturnValue<Object> stun_header(const Env& env, const stun::Header& hdr) noexcept;

}
