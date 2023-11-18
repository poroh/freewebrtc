//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Defines possibility of specify dynamic hash function
// for unordered_map/unordered_set
//

#pragma once

#include <cstddef>

namespace freewebrtc::util::hash::dynamic {

template<typename HashedType>
class Algorithm {
public:
    virtual ~Algorithm() = 0;
    virtual std::size_t hash(const HashedType&) const noexcept = 0;
};

template<typename HashedType, typename AlgoInstance>
struct AlgorithmSingleton {
    using Algo = Algorithm<HashedType>;
    static Algo& get() noexcept;
};

template<typename HashedType>
class Hash {
public:
    Hash(const Hash&) = default;
    Hash(Hash&&) = default;
    Hash& operator=(const Hash&) = default;
    Hash& operator=(Hash&&) = default;

    template<typename AlgoSingleton>
    static Hash create();

    std::size_t operator()(const HashedType& key) const noexcept;
private:
    using Algo = Algorithm<HashedType>;

    template<typename AlgoSingleton>
    Hash(AlgoSingleton);
    Algo& m_algo;
};

//
// implementation
//
template<typename T>
Algorithm<T>::~Algorithm()
{}

template<typename T, typename I>
Algorithm<T>& AlgorithmSingleton<T, I>::get() noexcept {
    static I instance;
    return instance;
}

template<typename T>
template<typename AlgoSingleton>
Hash<T> Hash<T>::create() {
    AlgoSingleton s;
    return Hash<T>(s);
}

template<typename T>
template<typename AlgoSingleton>
Hash<T>::Hash(AlgoSingleton)
    : m_algo(AlgoSingleton::get())
{}

template<typename T>
std::size_t Hash<T>::operator()(const T& v) const noexcept {
    return m_algo.hash(v);
}

}

