//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Parsing statistics
//

#pragma once

#include "stat/stat_counter.hpp"

namespace freewebrtc::stun {

struct ParseStat {
    stat::Counter success;
    stat::Counter error;
    stat::Counter invalid_size;
    stat::Counter invalid_attr_size;
};

}
