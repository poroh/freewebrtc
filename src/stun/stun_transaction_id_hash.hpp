//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Transaction Identifier Hash
//

#pragma once

#include "stun/stun_transaction_id.hpp"
#include "util/util_hash_dynamic.hpp"
#include "util/util_hash_murmur.hpp"

namespace freewebrtc::stun {

using TransactionIdHash = util::hash::dynamic::Hash<TransactionId>;

template<typename RandomGen>
class MurmurTransactionIdHash : public util::hash::dynamic::Algorithm<TransactionId> {
public:
    MurmurTransactionIdHash();
    std::size_t hash(const TransactionId&) const noexcept override;
    static TransactionIdHash create() noexcept;
private:
    size_t m_seed;
};

template<typename RandomGen>
MurmurTransactionIdHash<RandomGen>::MurmurTransactionIdHash()
    : m_seed(0)
{
    RandomGen r;
    m_seed = r();
}

template<typename RandomGen>
std::size_t MurmurTransactionIdHash<RandomGen>::hash(const TransactionId& id) const noexcept {
    return util::hash::murmur(id.view(), m_seed);
}

template<typename RandomGen>
TransactionIdHash MurmurTransactionIdHash<RandomGen>::create() noexcept {
    using Singleton = util::hash::dynamic::AlgorithmSingleton<TransactionId, MurmurTransactionIdHash>;
    return TransactionIdHash::create<Singleton>();
}

}
