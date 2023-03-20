//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Binary packet view (does not contain own data, just works over existing buffer)
//

#pragma once

#include <arpa/inet.h>

namespace freewebrtc::util {

uint32_t network_to_host_u32(uint32_t v);
uint16_t network_to_host_u16(uint16_t v);

uint32_t host_to_network_u32(uint32_t v);
uint16_t host_to_network_u16(uint16_t v);

//
// inlines
//
inline uint32_t network_to_host_u32(uint32_t v) {
    return ntohl(v);
}

inline uint16_t network_to_host_u16(uint16_t v) {
    return ntohs(v);
}

inline uint32_t host_to_network_u32(uint32_t v) {
    return htonl(v);
}

inline uint16_t host_to_network_u16(uint16_t v) {
    return htons(v);
}

}
