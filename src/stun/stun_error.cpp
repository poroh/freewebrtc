//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Errors
//

#include "stun/stun_error.hpp"

namespace freewebrtc::stun {

class ParseErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "stun parse error";
    }
    std::string message(int code) const override {
        switch ((ParseError)code) {
        case ParseError::ok:                        return "success";
        case ParseError::invalid_message_size:      return "invalid message size";
        case ParseError::not_padded_attributes:     return "not padded attributes";
        case ParseError::invalid_message_len:       return "invalid message length";
        case ParseError::invalid_magic_cookie:      return "invalid magic cookie";
        case ParseError::invalid_attr_size:         return "invalid attr size";
        case ParseError::invalid_mapped_addr:       return "invalid mapped address";
        case ParseError::invalid_xor_mapped_addr:   return "invalid xor mapped address";
        case ParseError::invalid_ipv4_address_size: return "invalid ipv4 address size";
        case ParseError::invalid_ipv6_address_size: return "invalid ipv6 address size";
        case ParseError::integrity_digest_size:     return "invalid integrity digest size";
        case ParseError::fingerprint_crc_size:      return "invalid fingerprint crc size";
        case ParseError::fingerprint_is_not_last:   return "fingerprint attribute is not the last";
        case ParseError::fingerprint_not_valid:     return "fingerprint is not valid";
        case ParseError::priority_attribute_size:   return "priority attribute size";
        case ParseError::ice_controlled_size:       return "invalid ice controlled attribute size";
        case ParseError::ice_controlling_size:      return "invalid ice controlling attribute size";
        case ParseError::use_candidate_size:        return "invalid use candidate size";
        case ParseError::error_code_attribute_size: return "invalid error code attribute size";
        case ParseError::unknown_addr_family:       return "unknown addr family";
        }
        return "unknown stun error";
    }
};

class ClientErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "stun client error";
    }
    std::string message(int code) const override {
        switch ((ClientError)code) {
        case ClientError::ok:                                 return "success";
        case ClientError::no_integrity_attribute_in_response: return "no expect integrity attribute in response";
        case ClientError::digest_is_not_valid:                return "digest is not valid in response";
        case ClientError::transaction_not_found:              return "transaction of response is not found";
        case ClientError::no_address_in_response:             return "bad response: no address in response";
        case ClientError::no_error_code_in_response:          return "bad response: no error code attribute in response";
        case ClientError::no_alternate_server_in_response:    return "bad response: no alternate server in 300 response";
        }
        return "unknown stun client error";
    }
};

const std::error_category& stun_parse_error_category() {
    static const ParseErrorCategory cat;
    return cat;
}


const std::error_category& stun_client_error_category() {
    static const ClientErrorCategory cat;
    return cat;
}

}
