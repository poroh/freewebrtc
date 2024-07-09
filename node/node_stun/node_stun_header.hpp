//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Mapping of STUN to node.js type
//

#pragma once

#include <craftnapi/env.hpp>
#include "stun/stun_header.hpp"
#include "util/util_result.hpp"

namespace freewebrtc::node_stun {

Result<craftnapi::Object> header(const craftnapi::Env& env, const stun::Header& hdr) noexcept;

}
