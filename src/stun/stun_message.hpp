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
#include "util/util_result.hpp"

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
    Maybe<util::ConstBinaryView::Interval> integrity_interval;
    // Parse message from binary view
    static Result<Message> parse(const util::ConstBinaryView&, ParseStat&);
    // Check that MESSAGE-INTEGRITY is valid (if present).
    // If MESSAGE-INTEGRITY is not present then function returns std::nullopt
    // Error may occue if hash function returns error. Otherwise return_value.value()
    // is always not nullptr
    Result<Maybe<bool>> is_valid(const util::ConstBinaryView&, const IntegrityData&) const noexcept;

    // Build message as bytes
    Result<util::ByteVec> build(const MaybeIntegrity& maybe_integrity = None{}) const noexcept;

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
    return attribute_set.error_code()
        .fmap([](auto&& ecref) {
            return ecref.get().code == ErrorCodeAttribute::TryAlternate;
        })
        .value_or(false);
}

}
