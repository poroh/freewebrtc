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

namespace freewebrtc::util {

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

}
