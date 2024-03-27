//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Fully Qualified Domain Name Tests
//

#include <gtest/gtest.h>

#include "net/net_fqdn.hpp"

namespace freewebrtc::test {

class NetFqdnTest : public ::testing::Test {
};

// ================================================================================
// Positive cases

TEST_F(NetFqdnTest, rfc1035_examples_tests) {
    std::vector<std::string_view> vec = {
        "A.ISI.EDU",
        "XX.LCS.MIT.EDU",
        "SRI-NIC.ARPA",
    };

    for (const auto& v: vec) {
        auto rv = net::Fqdn::parse(v);
        ASSERT_TRUE(rv.is_ok());
        auto parse_success = rv.unwrap();
        EXPECT_EQ(parse_success.value.to_string(), v);
        EXPECT_TRUE(parse_success.rest.empty());
    }
}

TEST_F(NetFqdnTest, with_final_dot) {
    std::vector<std::string_view> vec = {
        "example.com.",
        "com."
    };

    for (const auto& v: vec) {
        auto rv = net::Fqdn::parse(v);
        ASSERT_TRUE(rv.is_ok());
        auto parse_success = rv.unwrap();
        EXPECT_EQ(parse_success.value.to_string(), v);
        EXPECT_TRUE(parse_success.rest.empty());
    }
}

TEST_F(NetFqdnTest, with_hyphens) {
    std::vector<std::string_view> vec = {
        "exa-mple.com.",
        "10.c--m"
    };

    for (const auto& v: vec) {
        auto rv = net::Fqdn::parse(v);
        ASSERT_TRUE(rv.is_ok());
        auto parse_success = rv.unwrap();
        EXPECT_EQ(parse_success.value.to_string(), v);
        EXPECT_TRUE(parse_success.rest.empty());
    }
}

TEST_F(NetFqdnTest, with_underscores) {
    std::vector<std::string_view> vec = {
        "exa_mple.com.",
        "exa_mple_.com",
        "exa_mple_.com.",
        "_.com",
        "_._.com",
    };

    for (const auto& v: vec) {
        auto rv = net::Fqdn::parse(v);
        ASSERT_TRUE(rv.is_ok());
        auto parse_success = rv.unwrap();
        EXPECT_EQ(parse_success.value.to_string(), v);
        EXPECT_TRUE(parse_success.rest.empty());
    }
}

TEST_F(NetFqdnTest, with_rest) {
    std::vector<std::tuple<std::string_view, std::string_view, std::string_view>> vec = {
        {"example.com-", "example.com", "-"},
        {"example.com?abc=1234", "example.com", "?abc=1234"},
        {"_example.com_", "_example.com_", ""},
    };

    for (const auto& t: vec) {
        auto rv = net::Fqdn::parse(std::get<0>(t));
        ASSERT_TRUE(rv.is_ok());
        auto parse_success = rv.unwrap();
        EXPECT_EQ(parse_success.value.to_string(), std::get<1>(t));
        EXPECT_EQ(parse_success.rest, std::get<2>(t));
    }
}

// ================================================================================
// Negative cases

TEST_F(NetFqdnTest, failed_to_parse) {
    std::vector<std::string_view> vec = {
        ".",
        "*example.com",
        "-example.com",
        "",
        " "
    };

    for (const auto& v: vec) {
        EXPECT_TRUE(net::Fqdn::parse(v).is_err());
    }
}

}

