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

uint64_t network_to_host_u64(uint64_t v);
uint32_t network_to_host_u32(uint32_t v);
uint16_t network_to_host_u16(uint16_t v);

uint64_t host_to_network_u64(uint64_t v);
uint32_t host_to_network_u32(uint32_t v);
uint16_t host_to_network_u16(uint16_t v);

//
// inlines
//
inline uint64_t network_to_host_u64(uint64_t v) {
    if (host_to_network_u16(0x1) == 0x01) {
        // big endian system
        return v;
    }
    const uint32_t high_part = network_to_host_u32(v >> 32);
    const uint32_t low_part = network_to_host_u32(v & 0xFFFFFFFFLL);
    return (static_cast<uint64_t>(low_part) << 32) | high_part;
}

inline uint32_t network_to_host_u32(uint32_t v) {
    return ntohl(v);
}

inline uint16_t network_to_host_u16(uint16_t v) {
    return ntohs(v);
}

inline uint64_t host_to_network_u64(uint64_t v) {
    if (host_to_network_u16(0x1) == 0x01) {
        // big endian system
        return v;
    }
    const uint32_t high_part = host_to_network_u32(v >> 32);
    const uint32_t low_part = host_to_network_u32(v & 0xFFFFFFFFLL);
    return (static_cast<uint64_t>(low_part) << 32) | high_part;
}

inline uint32_t host_to_network_u32(uint32_t v) {
    return htonl(v);
}

inline uint16_t host_to_network_u16(uint16_t v) {
    return htons(v);
}

}
