//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Method
//

#include "stun/stun_method.hpp"
#include "stun/details/stun_method_registry.hpp"

namespace freewebrtc::stun {

Method Method::binding() noexcept {
    return Method(method_registry::BINDING);
}


}
