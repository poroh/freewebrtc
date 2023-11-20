//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN client transaction handle
//

#pragma once

#include <stddef.h>
#include <functional>

namespace freewebrtc::stun::client_udp {

struct Handle {
    unsigned value;
    bool operator<=>(const Handle&) const noexcept = default;
};

struct HandleHash {
    size_t operator()(Handle) const noexcept;
};

//
// inlines
//
inline size_t HandleHash::operator()(Handle hnd) const noexcept {
    std::hash<decltype(hnd.value)> h;
    return h(hnd.value);
}

}


