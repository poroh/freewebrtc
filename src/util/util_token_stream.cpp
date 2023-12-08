//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// String-based token stream for parsers
//

#include "util/util_token_stream.hpp"

namespace freewebrtc::util {

namespace {

class TokenStreamErrorCategory : public std::error_category {
public:
    const char *name() const noexcept override;
    std::string message(int code) const override;
};

const std::error_category& token_stream_error_category() noexcept {
    static const TokenStreamErrorCategory cat;
    return cat;
}

const char *TokenStreamErrorCategory::name() const noexcept {
    return "token stream error";
}

std::string TokenStreamErrorCategory::message(int code) const {
    switch ((TokenStream::Error)code) {
    case TokenStream::Error::ok: return "Success";
    case TokenStream::Error::no_required_token: return "No required token";
    case TokenStream::Error::expected_token_missed: return "Expected token is missed";
    }
    return "Unknown token stream error: " + std::to_string(code);
}

}

TokenStream::TokenStream(const Container& d)
    : m_data(d)
    , m_pos(m_data.cbegin())
{}

TokenStream::TokenStream(Container&& d)
    : m_data(std::move(d))
    , m_pos(m_data.cbegin())
{}


::freewebrtc::Error TokenStream::make_error_code(Error code) {
    return std::error_code(static_cast<int>(code), token_stream_error_category());
}

}
