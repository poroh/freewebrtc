//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Binary packet view (does not contain own data, just works over existing buffer)
//

#pragma once

#include <craftpp/binary/const_view.hpp>

namespace freewebrtc::util {

using ConstBinaryView = craftpp::binary::ConstView;
using ByteVec = ConstBinaryView::ByteVec;

}
