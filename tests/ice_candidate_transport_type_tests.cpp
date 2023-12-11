//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE candidate type tests
//

#include <gtest/gtest.h>
#include "ice/candidate/ice_candidate_transport_type.hpp"
#include "ice/candidate/ice_candidate_error.hpp"

namespace freewebrtc::tests {

class IceCandidateTransportTypeTest : public ::testing::Test {
public:
    using TransportType = ice::candidate::TransportType;
};

TEST_F(IceCandidateTransportTypeTest, from_string_tests) {
    // RFC8839:
    // This specification only defines UDP. However, extensibility is
    // provided to allow for future transport protocols to be used
    // with ICE by extending the subregistry "ICE Transport Protocols"
    // under the "Interactive Connectivity Establishment (ICE)"
    // registry.
    const std::vector<std::string_view> all { "UDP", "Udp", "uDp", "udp" };
    for (auto t: all) {
        ASSERT_TRUE(TransportType::from_string(t).is_value());
    }
    EXPECT_EQ(TransportType::from_string("UDP").assert_value(), TransportType::udp());
}

TEST_F(IceCandidateTransportTypeTest, from_unknown_string_tests) {
    ASSERT_TRUE(TransportType::from_string("unknown_string").is_error());
    ASSERT_EQ(TransportType::from_string("unknown_string").assert_error().value(), (int)ice::candidate::Error::unknown_transport_type);
}

}
