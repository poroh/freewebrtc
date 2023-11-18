//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Message functions
//

#pragma once

#include <vector>
#include <system_error>

#include "util/util_binary_view.hpp"
#include "util/util_typed_bool.hpp"
#include "util/util_return_value.hpp"

#include "stun/stun_header.hpp"
#include "stun/stun_attribute_set.hpp"
#include "stun/stun_parse_stat.hpp"
#include "stun/stun_integrity.hpp"

#include "crypto/crypto_hash.hpp"

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
    std::optional<util::ConstBinaryView::Interval> integrity_interval;
    // Parse message from binary view
    static ReturnValue<Message> parse(const util::ConstBinaryView&, ParseStat&);
    // Check that MESSAGE-INTEGRITY is valid (if present).
    // If MESSAGE-INTEGRITY is not present then function returns std::nullopt
    // Error may occue if hash function returns error. Otherwise return_value.value()
    // is always not nullptr
    ReturnValue<std::optional<bool>> is_valid(const util::ConstBinaryView&, const IntegrityData&) const noexcept;

    // Build message as bytes
    ReturnValue<util::ByteVec> build(const MaybeIntegrity& maybe_integrity = std::nullopt) const noexcept;

    // Check if message is alternate server response
    bool is_alternate_server() const noexcept;
};

//
// inlines
//
inline bool Message::is_alternate_server() const noexcept {
    if (header.cls != Class::error_response()) {
        return false;
    }
    auto maybe_error = attribute_set.error_code();
    return maybe_error.has_value()
        && maybe_error.value().get().code == ErrorCodeAttribute::TryAlternate;
}

}
