//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Code from https://en.cppreference.com/w/cpp/utility/variant/visit example
// to use lambda-like matching for variants
//
//

#pragma once

#include <string_view>
#include <utility>
#include <sstream>
#include <vector>
#include "util/util_maybe.hpp"

namespace freewebrtc::util::string_view {

using SV = std::string_view;
using MaybeSV = Maybe<SV>;

// Safe version of std::string_view::remote_prefix
MaybeSV remote_prefix(SV, size_t sz);

Maybe<std::pair<SV, SV>> split(SV, char sep);

template<template <typename...> class Result = std::vector>
Result<SV> split_all(SV, char sep);

template<typename Container>
std::string join(const Container&, const std::string& sep);

//
// implementation
//
inline MaybeSV remove_prefix(SV sv, size_t sz) {
    if (sz > sv.size()) {
        return None{};
    }
    sv.remove_prefix(sz);
    return sv;
}

inline Maybe<std::pair<SV, SV>> split(SV sv, char sep) {
    if (auto pos = sv.find(sep); pos != std::string_view::npos) {
        auto first = SV{sv.data(), pos};
        return remove_prefix(sv, pos + 1)
            .fmap([&](const SV& second) {
                return std::make_pair(std::move(first), std::move(second));
            });
    }
    return None{};
}

template<template <typename...> class Result>
Result<SV> split_all(SV sv, char sep) {
    Result<SV> result;
    while (!sv.empty()) {
        auto maybe_pair = split(sv, sep);
        if (maybe_pair.is_some()) {
            auto& pair = maybe_pair.unwrap();
            result.emplace_back(std::move(pair.first));
            sv = pair.second;
        } else {
            result.emplace_back(std::move(sv));
            sv = SV{};
        }
    }
    return result;
}

template<typename Container>
std::string join(const Container& c, const std::string& sep) {
    std::ostringstream sstr;
    for (auto it = c.begin(); it != c.end(); ++it) {
        sstr << (it == c.begin() ? "" : sep) << *it;
    }
    return sstr.str();
}


}
