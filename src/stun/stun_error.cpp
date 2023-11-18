//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Errors
//

#include "stun/stun_error.hpp"

namespace freewebrtc::stun {

class ErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "stun error";
    }
    std::string message(int code) const override {
        switch ((Error)code) {
        case Error::OK:                        return "success";
        case Error::INVALID_MESSAGE_SIZE:      return "invalid message size";
        case Error::NOT_PADDED_ATTRIBUTES:     return "not padded attributes";
        case Error::INVALID_MESSAGE_LEN:       return "invalid message length";
        case Error::INVALID_MAGIC_COOKIE:      return "invalid magic cookie";
        case Error::INVALID_ATTR_SIZE:         return "invalid attr size";
        case Error::INVALID_MAPPED_ADDR:       return "invalid mapped address";
        case Error::INVALID_XOR_MAPPED_ADDR:   return "invalid xor mapped address";
        case Error::INVALID_IPV4_ADDRESS_SIZE: return "invalid ipv4 address size";
        case Error::INVALID_IPV6_ADDRESS_SIZE: return "invalid ipv6 address size";
        case Error::INTEGRITY_DIGEST_SIZE:     return "invalid integrity digest size";
        case Error::FINGERPRINT_CRC_SIZE:      return "invalid fingerprint crc size";
        case Error::FINGERPRINT_IS_NOT_LAST:   return "fingerprint attribute is not the last";
        case Error::FINGERPRINT_NOT_VALID:     return "fingerprint is not valid";
        case Error::PRIORITY_ATTRIBUTE_SIZE:   return "priority attribute size";
        case Error::ICE_CONTROLLED_SIZE:       return "invalid ice controlled attribute size";
        case Error::ICE_CONTROLLING_SIZE:      return "invalid ice controlling attribute size";
        case Error::USE_CANDIDATE_SIZE:        return "invalid use candidate size";
        case Error::ERROR_CODE_ATTRIBUTE_SIZE: return "invalid error code attribute size";
        case Error::UNKNOWN_ADDR_FAMILY:       return "unknown addr family";
        }
        return "unknown stun error";
    }
};

const std::error_category& stun_error_category() {
    static const ErrorCategory cat;
    return cat;
}



}
