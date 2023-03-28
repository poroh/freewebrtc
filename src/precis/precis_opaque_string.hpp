//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// OpaqueString is profile for strings that is defined
// by PRECIS framework
// Specifically see:
// https://www.rfc-editor.org/rfc/rfc8265#section-4.2
//

#pragma once

#include <string>
#include "util/util_tagged_type.hpp"

namespace freewebrtc::precis {

// Note: Today we don't have function of generation
// of OpaqueString and it is not simple function that requires
// Normalization Form C transform to make two strings
// comparable to each other. In future I'm going to provide
// wrapper that is using ICU wrapper.
//
// See: https://www.unicode.org/reports/tr15/ for more details.
//
struct OpaqueStringTag;
using OpaqueString = util::TaggedType<std::string, OpaqueStringTag>;

}
