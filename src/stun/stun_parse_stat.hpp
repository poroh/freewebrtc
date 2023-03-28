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
    stat::Counter not_padded;
    stat::Counter message_length_error;
    stat::Counter magic_cookie_error;
    stat::Counter invalid_attr_size;
    stat::Counter fingerprint_not_last;
    stat::Counter invalid_fingerprint;
    stat::Counter invalid_fingerprint_size;
    stat::Counter invalid_message_integrity;
    stat::Counter invalid_xor_mapped_address;
    stat::Counter invalid_ip_address;
    stat::Counter unknown_comprehension_required_attr;
};

}
