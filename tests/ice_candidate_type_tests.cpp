//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE candidate type tests
//

#include <gtest/gtest.h>
#include "ice/candidate/ice_candidate_type.hpp"
#include "ice/candidate/ice_candidate_error.hpp"

namespace freewebrtc::tests {

class IceCandidateTypeTest : public ::testing::Test {
public:
    using Type = ice::candidate::Type;
};

TEST_F(IceCandidateTypeTest, from_string_tests) {
    // RFC8839:
    // This specification defines the values "host", "srflx",
    // "prflx", and "relay" for host, server-reflexive,
    // peer-reflexive, and relayed candidates, respectively.
    const std::vector<std::string_view> all { "host", "srflx", "prflx", "relay" };
    for (auto t: all) {
        ASSERT_TRUE(Type::from_string(t).is_value());
    }
    EXPECT_EQ(Type::from_string("host").assert_value(),  Type::host());
    EXPECT_EQ(Type::from_string("srflx").assert_value(), Type::server_reflexive());
    EXPECT_EQ(Type::from_string("prflx").assert_value(), Type::peer_reflexive());
    EXPECT_EQ(Type::from_string("relay").assert_value(), Type::relayed());
}

TEST_F(IceCandidateTypeTest, from_unknown_string_tests) {
    ASSERT_TRUE(Type::from_string("unknown_string").is_error());
    ASSERT_EQ(Type::from_string("unknown_string").assert_error().value(), (int)ice::candidate::Error::unknown_candidate_type);
}

}
