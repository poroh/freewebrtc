//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Marker bit
//

#pragma once

#include "util/UtilTypedBool.hpp"

namespace freewebrtc::rtp {

struct MarkerTag;
using MarkerBit = util::TypedBool<MarkerTag>;

};
