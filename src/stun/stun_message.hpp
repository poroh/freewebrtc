//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Message functions
//

#pragma once

#include <vector>

#include "stun/stun_header.hpp"
#include "stun/stun_attribute.hpp"
#include "util/util_binary_view.hpp"
#include "stun/stun_parse_stat.hpp"

namespace freewebrtc::stun {

// All STUN messages MUST start with a 20-byte header followed by zero
// or more Attributes.
struct Message {
    Header header;
    std::vector<Attribute> attributes;
    // Data interval that is covered by MESSAGE-INTEGRITY attribute (if any).
    std::optional<util::ConstBinaryView::Interval> integrity;
    std::optional<Message> parse(const util::ConstBinaryView&, ParseStat&);
};

}
