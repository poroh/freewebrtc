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

enum class ParseError {
    ok = 0,
    invalid_message_size,
    not_padded_attributes,
    invalid_message_len,
    invalid_magic_cookie,
    invalid_attr_size,
    invalid_mapped_addr,
    invalid_xor_mapped_addr,
    invalid_ipv4_address_size,
    invalid_ipv6_address_size,
    integrity_digest_size,
    fingerprint_crc_size,
    fingerprint_is_not_last,
    fingerprint_not_valid,
    priority_attribute_size,
    ice_controlled_size,
    ice_controlling_size,
    use_candidate_size,
    error_code_attribute_size,
    unknown_attributes_attribute_size,
    unknown_addr_family
};

enum class ClientError {
    ok = 0,
    no_integrity_attribute_in_response,
    digest_is_not_valid,
    transaction_not_found,
    no_address_in_response,
    no_error_code_in_response,
    no_alternate_server_in_response
};

std::error_code make_error_code(ParseError);
std::error_code make_error_code(ClientError);

const std::error_category& stun_parse_error_category();
const std::error_category& stun_client_error_category();

//
// inline
//
inline std::error_code make_error_code(ParseError ec) {
    return std::error_code((int)ec, stun_parse_error_category());
}

inline std::error_code make_error_code(ClientError ec) {
    return std::error_code((int)ec, stun_client_error_category());
}


}
