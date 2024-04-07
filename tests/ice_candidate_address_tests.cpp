//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE candidate type tests
//

#include <gtest/gtest.h>
#include "ice/candidate/ice_candidate_address.hpp"
#include "ice/candidate/ice_candidate_error.hpp"

namespace freewebrtc::tests {

class IceCandidateAddressTest : public ::testing::Test {
public:
    using Address = ice::candidate::Address;
};

TEST_F(IceCandidateAddressTest, ipv4_from_string_test) {
    const char *ipv4_str = "192.0.2.3";
    auto ipv4_rv = Address::from_string(ipv4_str);
    ASSERT_TRUE(ipv4_rv.is_ok());
    ASSERT_TRUE(ipv4_rv.unwrap().as_ip_address().is_some());
    EXPECT_EQ(ipv4_rv.unwrap().as_ip_address().unwrap().get(), net::ip::Address::from_string(ipv4_str).unwrap());
}

TEST_F(IceCandidateAddressTest, ipv6_from_string_test) {
    const char *ipv6_str = "fe80::6676:baff:fe9c:ee4a";
    auto ipv6_rv = Address::from_string(ipv6_str);
    ASSERT_TRUE(ipv6_rv.is_ok());
    ASSERT_TRUE(ipv6_rv.unwrap().as_ip_address().is_some());
    EXPECT_EQ(ipv6_rv.unwrap().as_ip_address().unwrap().get(), net::ip::Address::from_string(ipv6_str).unwrap());
}

TEST_F(IceCandidateAddressTest, fqdn_from_string_test) {
    const char fqdn_str[] = "example.com";
    auto fqdn_rv = Address::from_string(fqdn_str);
    ASSERT_TRUE(fqdn_rv.is_ok());
    ASSERT_TRUE(fqdn_rv.unwrap().as_fqdn().is_some());
    EXPECT_EQ(fqdn_rv.unwrap().as_fqdn().unwrap().get(), net::Fqdn::from_string(fqdn_str).unwrap());
}

TEST_F(IceCandidateAddressTest, failed_to_parse) {
    std::vector<std::string> parse_errors = {
        "",
        "@",
        ".",
        "192.168.1.2:123"
    };
    for (const auto& str: parse_errors) {
        auto fqdn_rv = Address::from_string(str);
        EXPECT_TRUE(fqdn_rv.is_err());
    }
}

}
