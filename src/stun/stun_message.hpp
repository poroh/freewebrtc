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
#include "stun/stun_attribute_set.hpp"
#include "util/util_binary_view.hpp"
#include "util/util_typed_bool.hpp"
#include "stun/stun_parse_stat.hpp"

namespace freewebrtc::stun {

struct IsRFC3489Tag;
using IsRFC3489 = util::TypedBool<IsRFC3489Tag>;


// All STUN messages MUST start with a 20-byte header followed by zero
// or more Attributes.
struct Message {
    Header header;
    AttributeSet attribute_set;
    // Mode for compatibility to RFC3489 (no magic cookie in request).
    IsRFC3489 is_rfc3489;
    // Data interval that is covered by MESSAGE-INTEGRITY attribute (if any).
    std::optional<util::ConstBinaryView::Interval> integrity;
    static std::optional<Message> parse(const util::ConstBinaryView&, ParseStat&);
};

}
