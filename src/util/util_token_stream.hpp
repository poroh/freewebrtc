//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// String-based token stream for parsers
//

#pragma once

#include <string_view>
#include <vector>
#include <type_traits>

#include "util/util_result.hpp"
#include "util/util_maybe.hpp"

namespace freewebrtc::util {

class TokenStream {
public:
    enum class Error {
        ok = 0,
        no_required_token,
        expected_token_missed
    };
    using TokenType = std::string_view;
    using Container = std::vector<TokenType>;
    using MaybToken =  Maybe<TokenType>;

    TokenStream(const Container&);
    TokenStream(Container&&);

    // Read next required token and transform it with function F
    template<typename F>
    auto required_bind(F&& f) noexcept -> Result<typename std::invoke_result_t<F, const TokenType&>::Value>;

    Result<TokenType> required() noexcept;
    MaybeError required(const std::string_view&) noexcept;

    MaybToken optional() noexcept;

private:
    ::freewebrtc::Error make_error_code(Error);

    const Container m_data;
    Container::const_iterator m_pos;
};


//
// implementation
//
template<typename F>
auto TokenStream::required_bind(F&& f) noexcept -> Result<typename std::invoke_result_t<F, const TokenType&>::Value> {
    return required().bind(f);
}

inline MaybeError TokenStream::required(const std::string_view& expected) noexcept {
    return required_bind([&](const std::string_view& v) {
        if (v == expected) {
            return success();
        }
        return MaybeError{ make_error_code(Error::expected_token_missed) }
            .add_context("expected", expected, "received", v);
    });
}

inline Result<TokenStream::TokenType> TokenStream::required() noexcept {
    if (m_pos == m_data.end()) {
        return make_error_code(Error::no_required_token);
    }
    return Result<TokenType>{*(m_pos++)};
}


}



