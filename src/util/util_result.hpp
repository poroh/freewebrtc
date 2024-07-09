//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Helper to handle either return value or std::error_code
//

#pragma once

#include <craftpp/result/result.hpp>

namespace freewebrtc {

using craftpp::result::Result;
using craftpp::result::MaybeError;
using craftpp::result::success;
using craftpp::result::return_value;

}
