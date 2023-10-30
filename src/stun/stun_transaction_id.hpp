//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Transaction Identifier
//

#pragma once

#include <array>
#include "util/util_binary_view.hpp"
#include "details/stun_constants.hpp"

namespace freewebrtc::stun {

class TransactionId {
public:
    explicit TransactionId(const util::ConstBinaryView&);
    TransactionId(TransactionId&&) = default;
    TransactionId(const TransactionId&) = default;

    template<typename RandomGen>
    static TransactionId generate(RandomGen&);

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

template<typename RandomGen>
TransactionId TransactionId::generate(RandomGen& r) {
    using W = uint32_t;
    std::array<W, details::TRANSACTION_ID_SIZE / sizeof(W)> value;
    for (auto& v: value) {
        v = r();
    }
    return TransactionId(util::ConstBinaryView(value.data(), value.size() * sizeof(W)));
}


}
