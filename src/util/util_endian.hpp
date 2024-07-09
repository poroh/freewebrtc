//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Binary packet view (does not contain own data, just works over existing buffer)
//

#pragma once

#include <craftpp/binary/endian.hpp>

namespace freewebrtc::util {

using craftpp::binary::network_to_host_u64;
using craftpp::binary::network_to_host_u32;
using craftpp::binary::network_to_host_u16;
using craftpp::binary::host_to_network_u64;
using craftpp::binary::host_to_network_u32;
using craftpp::binary::host_to_network_u16;



}
