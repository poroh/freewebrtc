//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Integrity Check data
//

#pragma once

#include "util/util_maybe.hpp"
#include "stun/stun_password.hpp"

namespace freewebrtc::stun {

struct IntegrityData {
    Password password;
    crypto::SHA1Hash::Func hash;
};

using MaybeIntegrity = Maybe<IntegrityData>;


}
