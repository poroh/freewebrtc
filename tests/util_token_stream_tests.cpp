//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Token stream tests
//

#include <gtest/gtest.h>
#include "util/util_token_stream.hpp"

namespace freewebrtc::tests {

class UtilTokenStreamTest : public ::testing::Test {
public:
    using TokenStream = util::TokenStream;
};

TEST_F(UtilTokenStreamTest, constructor_const_vector) {
    std::vector<std::string_view> tokens = {"token1", "token2", "token3"};
    TokenStream tstr(tokens);
}

TEST_F(UtilTokenStreamTest, constructor_rvalue_vector) {
    TokenStream tstr(std::vector<std::string_view>{"token1", "token2", "token3"});
}

TEST_F(UtilTokenStreamTest, required_bind_success) {
    std::vector<std::string_view> tokens = {"token1"};
    TokenStream tstr(tokens);
    auto result = tstr.required_bind([](const std::string_view& token) {
        return Result<std::string>{std::string(token)};
    });
    ASSERT_TRUE(result.is_ok());
    ASSERT_EQ(result.unwrap(), "token1");
}

TEST_F(UtilTokenStreamTest, required_multiple) {
    std::vector<std::string_view> tokens = {"token1", "token2", "token3"};
    TokenStream tstr(tokens);
    EXPECT_EQ(tstr.required().unwrap(), tokens[0]);
    EXPECT_EQ(tstr.required().unwrap(), tokens[1]);
    EXPECT_EQ(tstr.required().unwrap(), tokens[2]);
}

TEST_F(UtilTokenStreamTest, required_bind_no_more_tokens) {
    TokenStream tstr(std::vector<std::string_view>{});
    auto result = tstr.required_bind([](const std::string_view& token) {
        return Result<std::string>{std::string(token)};
    });
    ASSERT_FALSE(result.is_ok());
    static_assert(std::is_same_v<decltype(result.unwrap()), std::string&>);
}

TEST_F(UtilTokenStreamTest, required_success) {
    std::vector<std::string_view> tokens = {"token1"};
    TokenStream tstr(tokens);
    auto result = tstr.required("token1");
    ASSERT_TRUE(result.is_ok());
}

TEST_F(UtilTokenStreamTest, required_unexpected_token) {
    std::vector<std::string_view> tokens = {"token1"};
    TokenStream tstr(tokens);
    auto result = tstr.required("token2");
    ASSERT_FALSE(result.is_ok());
}

TEST_F(UtilTokenStreamTest, required_no_more_tokens) {
    TokenStream tstr(std::vector<std::string_view>{});
    auto result = tstr.required("token1");
    ASSERT_FALSE(result.is_ok());
}

}

