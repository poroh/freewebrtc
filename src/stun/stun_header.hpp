//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Header
//

#pragma once

#include <vector>

#include "stun/stun_method.hpp"
#include "stun/stun_class.hpp"
#include "stun/stun_transaction_id.hpp"

namespace freewebrtc::stun {

struct Header {
    Class cls;
    Method method;
    TransactionId transaction_id;
};


}
