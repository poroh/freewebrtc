//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Common parse result
//

#pragma once

#include "util/util_result.hpp"

namespace freewebrtc {

template<typename V>
struct ParseSuccess {
    V value;
    std::string_view rest;
};

template<typename V>
using ParseResult = Result<ParseSuccess<V>>;

}


