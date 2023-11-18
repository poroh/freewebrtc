//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Errors
//

#pragma once

#include <system_error>

namespace freewebrtc::stun {

enum class Error {
    OK = 0,
    INVALID_MESSAGE_SIZE,
    NOT_PADDED_ATTRIBUTES,
    INVALID_MESSAGE_LEN,
    INVALID_MAGIC_COOKIE,
    INVALID_ATTR_SIZE,
    INVALID_MAPPED_ADDR,
    INVALID_XOR_MAPPED_ADDR,
    INVALID_IPV4_ADDRESS_SIZE,
    INVALID_IPV6_ADDRESS_SIZE,
    INTEGRITY_DIGEST_SIZE,
    FINGERPRINT_CRC_SIZE,
    FINGERPRINT_IS_NOT_LAST,
    FINGERPRINT_NOT_VALID,
    PRIORITY_ATTRIBUTE_SIZE,
    ICE_CONTROLLED_SIZE,
    ICE_CONTROLLING_SIZE,
    USE_CANDIDATE_SIZE,
    ERROR_CODE_ATTRIBUTE_SIZE,
    UNKNOWN_ADDR_FAMILY
};

std::error_code make_error_code(Error);

const std::error_category& stun_error_category();

//
// inline
//
inline std::error_code make_error_code(Error ec) {
    return std::error_code((int)ec, stun_error_category());
}


}
