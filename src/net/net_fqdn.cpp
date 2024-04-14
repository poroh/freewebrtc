//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Fully Qualified Domain Name
//

#include "net/net_fqdn.hpp"
#include "net/net_error.hpp"

namespace freewebrtc::net {

// RFC1035 2.3.1. Preferred name syntax
// The following syntax will result in fewer problems with many
// applications that use domain names (e.g., mail, TELNET).
// <domain> ::= <subdomain> | " "
// <subdomain> ::= <label> | <subdomain> "." <label>
// <label> ::= <letter> [ [ <ldh-str> ] <let-dig> ]
// <ldh-str> ::= <let-dig-hyp> | <let-dig-hyp> <ldh-str>
// <let-dig-hyp> ::= <let-dig> | "-"
// <let-dig> ::= <letter> | <digit>
// <letter> ::= any one of the 52 alphabetic characters A through Z in
// upper case and a through z in lower case
// <digit> ::= any one of the ten digits 0 through 9

namespace {

// ParseResult<std::string_view> parse_subdomain(std::string_view) {
//     //
// }

bool is_letter(char c) noexcept {
    return ('A' <= c && c <= 'Z')
        || ('a' <= c && c <= 'z')
        || c == '_'; // This is not standard however somebody can use it :(
}

bool is_digit(char c) noexcept {
    return '0' <= c && c <= '9';
}

ParseResult<std::string_view> parse_label(std::string_view v) {
    if (v.empty() || (!is_letter(v[0]) && !is_digit(v[0]))) {
        return make_error_code(Error::fqdn_invalid_label_expect_letter);
    }
    // Label is string that starts with letter, contains
    // alpha-digits and hyphen but does not end with hyphen
    using PosType = std::string_view::size_type;
    using SizeType = std::string_view::size_type;
    PosType last_let_dig = 0;
    PosType pos = 1;
    while (pos < v.size()) {
        const char c = v[pos];
        if (is_letter(c) || is_digit(c)) {
            last_let_dig = pos;
        } else if (c != '-') {
            break;
        }
        pos++;
    }
    const SizeType label_size = last_let_dig + 1;
    return ParseSuccess<std::string_view>{
        .value = std::string_view(v.data(), label_size),
        .rest  = std::string_view(v.data() + label_size, v.size() - label_size)
    };
}

ParseResult<std::string_view> parse_subdomain(std::string_view v) {
    // <subdomain> is label or <subdomain> . <label>
    // or: labels separated joined by dots.
    using SizeType = std::string_view::size_type;
    SizeType sz = 0;
    auto return_success = [&]() {
        return ParseSuccess<std::string_view>{
            .value = {v.data(), sz},
            .rest = {v.data() + sz, v.size() - sz}
        };
    };
    auto rest = v;

    while (true) {
        auto rv = parse_label(rest);
        if (rv.is_err()) {
            if (sz == 0) {
                return rv.unwrap_err();
            } else {
                return return_success();
            }
        }
        const auto& parse_success = rv.unwrap();
        sz += parse_success.value.size();
        rest = parse_success.rest;
        if (rest.empty()) {
            return return_success();
        }
        if (rest[0] != '.') {
            return return_success();
        }
        sz++;
        rest = {rest.data() + 1, rest.size() - 1};
    }
}

}

Fqdn::Fqdn(std::string_view v)
    : m_value(v)
{}

ParseResult<Fqdn> Fqdn::parse(std::string_view v) {
    return parse_subdomain(v)
        .fmap([](auto&& parse_success) {
            return ParseSuccess<Fqdn>{
                .value = Fqdn(parse_success.value),
                .rest = parse_success.rest
            };
        });
}

Result<Fqdn> Fqdn::from_string(std::string_view v) {
    return parse(v)
        .bind([](auto& parse_success) {
            using RetVal = Result<Fqdn>;
            if (parse_success.rest.empty()) {
                return RetVal{parse_success.value};
            }
            return RetVal{make_error_code(Error::fqdn_not_fully_parsed)};
        });
}

}

