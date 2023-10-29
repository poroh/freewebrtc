//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Transaction Identifier
//

#pragma once

#include "util/util_binary_view.hpp"

namespace freewebrtc::stun {

class TransactionId {
public:
    explicit TransactionId(const util::ConstBinaryView&);
    TransactionId(TransactionId&&) = default;
    util::ConstBinaryView view() const noexcept;
    bool operator==(const TransactionId&) const noexcept = default;
private:
    std::vector<uint8_t> m_value;
};

//
// inlines
//
inline TransactionId::TransactionId(const util::ConstBinaryView& vv)
    : m_value(vv.begin(), vv.end())
{}


inline util::ConstBinaryView TransactionId::view() const noexcept {
    return util::ConstBinaryView(m_value);
}

}
