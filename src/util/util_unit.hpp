//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Unit type
//

#pragma once

namespace freewebrtc {

struct Unit {
    using Self = Unit;
    static Self create() noexcept;
};

Unit unit() noexcept;

//
// implementation
//
inline Unit Unit::create() noexcept {
    return Self{};
}

inline Unit unit() noexcept {
    return Unit::create();
}

}

