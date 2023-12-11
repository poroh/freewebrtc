//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE candidate SDP attribute tests
//

#include <gtest/gtest.h>
#include "ice/candidate/ice_candidate_sdp.hpp"

namespace freewebrtc::tests {

class IceCandidateSDPAttrTests : public ::testing::Test {
public:
};

TEST_F(IceCandidateSDPAttrTests, parse_tests_rfc8839_example_host) {
    auto result_rv = ice::candidate::parse_sdp_attr("candidate:1 1 UDP 2130706431 203.0.113.141 8998 typ host");
    ASSERT_TRUE(result_rv.is_value());
    auto result = result_rv.assert_value();
    ASSERT_TRUE(std::holds_alternative<ice::candidate::Supported>(result));
    const auto& supported = std::get<ice::candidate::Supported>(result);
    const auto& candidate = supported.candidate;
    EXPECT_EQ(candidate.foundation, ice::candidate::Foundation::from_string("1").assert_value());
    ASSERT_TRUE(candidate.address.as_ip_address().has_value());
    EXPECT_EQ(candidate.address.as_ip_address()->get(), net::ip::Address::from_string("203.0.113.141").assert_value());
    EXPECT_EQ(candidate.port, net::Port::from_uint16(8998));
    EXPECT_EQ(candidate.transport_type, ice::candidate::TransportType::udp());
    EXPECT_EQ(candidate.type, ice::candidate::Type::host());
    EXPECT_EQ(candidate.priority, ice::candidate::Priority::from_uint32(2130706431).assert_value());
    ASSERT_TRUE(candidate.maybe_component.has_value());
    EXPECT_EQ(candidate.maybe_component.value(), ice::candidate::ComponentId::from_unsigned(1).assert_value());
    EXPECT_FALSE(candidate.maybe_related_address.has_value());
    EXPECT_FALSE(candidate.maybe_related_port.has_value());
}

TEST_F(IceCandidateSDPAttrTests, parse_tests_rfc8839_example_srflx) {
    auto result_rv = ice::candidate::parse_sdp_attr("candidate:2 1 UDP 1694498815 192.0.2.3 45664 typ srflx raddr 203.0.113.141 rport 8998");
    ASSERT_TRUE(result_rv.is_value());
    auto result = result_rv.assert_value();
    ASSERT_TRUE(std::holds_alternative<ice::candidate::Supported>(result));
    const auto& supported = std::get<ice::candidate::Supported>(result);
    const auto& candidate = supported.candidate;
    EXPECT_EQ(candidate.foundation, ice::candidate::Foundation::from_string("2").assert_value());
    ASSERT_TRUE(candidate.address.as_ip_address().has_value());
    EXPECT_EQ(candidate.address.as_ip_address()->get(), net::ip::Address::from_string("192.0.2.3").assert_value());
    EXPECT_EQ(candidate.port, net::Port::from_uint16(45664));
    EXPECT_EQ(candidate.transport_type, ice::candidate::TransportType::udp());
    EXPECT_EQ(candidate.type, ice::candidate::Type::server_reflexive());
    EXPECT_EQ(candidate.priority, ice::candidate::Priority::from_uint32(1694498815).assert_value());
    ASSERT_TRUE(candidate.maybe_component.has_value());
    EXPECT_EQ(candidate.maybe_component.value(), ice::candidate::ComponentId::from_unsigned(1).assert_value());
    ASSERT_TRUE(candidate.maybe_related_address.has_value());
    ASSERT_TRUE(candidate.maybe_related_port.has_value());
    ASSERT_TRUE(candidate.maybe_related_address->as_ip_address().has_value());
    auto& raddr = candidate.maybe_related_address->as_ip_address()->get();
    EXPECT_EQ(raddr, net::ip::Address::from_string("203.0.113.141").assert_value());
    EXPECT_EQ(candidate.maybe_related_port.value(), net::Port::from_uint16(8998));
}

TEST_F(IceCandidateSDPAttrTests, parse_tests_rfc8839_example_host_ipv6) {
    auto result_rv = ice::candidate::parse_sdp_attr("candidate:1 1 UDP 2130706431 fe80::6676:baff:fe9c:ee4a 8998 typ host");
    ASSERT_TRUE(result_rv.is_value());
    auto result = result_rv.assert_value();
    ASSERT_TRUE(std::holds_alternative<ice::candidate::Supported>(result));
    const auto& supported = std::get<ice::candidate::Supported>(result);
    const auto& candidate = supported.candidate;
    EXPECT_EQ(candidate.foundation, ice::candidate::Foundation::from_string("1").assert_value());
    ASSERT_TRUE(candidate.address.as_ip_address().has_value());
    EXPECT_EQ(candidate.address.as_ip_address()->get(), net::ip::Address::from_string("fe80::6676:baff:fe9c:ee4a").assert_value());
    EXPECT_EQ(candidate.port, net::Port::from_uint16(8998));
    EXPECT_EQ(candidate.transport_type, ice::candidate::TransportType::udp());
    EXPECT_EQ(candidate.type, ice::candidate::Type::host());
    EXPECT_EQ(candidate.priority, ice::candidate::Priority::from_uint32(2130706431).assert_value());
    ASSERT_TRUE(candidate.maybe_component.has_value());
    EXPECT_EQ(candidate.maybe_component.value(), ice::candidate::ComponentId::from_unsigned(1).assert_value());
    EXPECT_FALSE(candidate.maybe_related_address.has_value());
    EXPECT_FALSE(candidate.maybe_related_port.has_value());
}

TEST_F(IceCandidateSDPAttrTests, parse_tests_rfc8839_example_srflx_ipv6) {
    auto result_rv = ice::candidate::parse_sdp_attr("candidate:2 1 UDP 1694498815 2001:db8:8101:3a55:4858:a2a9:22ff:99b9 45664 typ srflx raddr fe80::6676:baff:fe9c:ee4a rport 8998");
    ASSERT_TRUE(result_rv.is_value());
    auto result = result_rv.assert_value();
    ASSERT_TRUE(std::holds_alternative<ice::candidate::Supported>(result));
    const auto& supported = std::get<ice::candidate::Supported>(result);
    const auto& candidate = supported.candidate;
    EXPECT_EQ(candidate.foundation, ice::candidate::Foundation::from_string("2").assert_value());
    ASSERT_TRUE(candidate.address.as_ip_address().has_value());
    EXPECT_EQ(candidate.address.as_ip_address()->get(), net::ip::Address::from_string("2001:db8:8101:3a55:4858:a2a9:22ff:99b9").assert_value());
    EXPECT_EQ(candidate.port, net::Port::from_uint16(45664));
    EXPECT_EQ(candidate.transport_type, ice::candidate::TransportType::udp());
    EXPECT_EQ(candidate.type, ice::candidate::Type::server_reflexive());
    EXPECT_EQ(candidate.priority, ice::candidate::Priority::from_uint32(1694498815).assert_value());
    ASSERT_TRUE(candidate.maybe_component.has_value());
    EXPECT_EQ(candidate.maybe_component.value(), ice::candidate::ComponentId::from_unsigned(1).assert_value());
    ASSERT_TRUE(candidate.maybe_related_address.has_value());
    ASSERT_TRUE(candidate.maybe_related_port.has_value());
    ASSERT_TRUE(candidate.maybe_related_address->as_ip_address().has_value());
    auto& raddr = candidate.maybe_related_address->as_ip_address()->get();
    EXPECT_EQ(raddr, net::ip::Address::from_string("fe80::6676:baff:fe9c:ee4a").assert_value());
    EXPECT_EQ(candidate.maybe_related_port.value(), net::Port::from_uint16(8998));
}

}
