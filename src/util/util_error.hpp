//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Error in freewebrtc library
//
// Motivation to implement this class is that
// std::error_code cannot provide context to error
// So it may be hard to narrow down in what context
// Error occur.
//

#pragma once

#include <craftpp/error/error.hpp>

namespace freewebrtc {

using craftpp::error::Error;

}
