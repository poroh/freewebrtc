//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE candidate foundation tests
//

#include <gtest/gtest.h>
#include "ice/candidate/ice_candidate_priority.hpp"

namespace freewebrtc::tests {

class IceCandidatePriorityTest : public ::testing::Test {
public:
    using Priority = ice::candidate::Priority;
};

TEST_F(IceCandidatePriorityTest, from_string_tests) {
    EXPECT_FALSE(Priority::from_string("0").is_value());
    EXPECT_FALSE(Priority::from_string("").is_value());
    EXPECT_TRUE(Priority::from_string("1").is_value());
    EXPECT_TRUE(Priority::from_string("4294967295").is_value());
    EXPECT_FALSE(Priority::from_string("4294967296").is_value());

    EXPECT_TRUE(Priority::from_string("90").is_value());
    EXPECT_EQ(Priority::from_string("1").assert_value(), Priority::from_uint32(1).assert_value());
    EXPECT_EQ(Priority::from_string("4294967295").assert_value(), Priority::from_uint32(4294967295).assert_value());
}

TEST_F(IceCandidatePriorityTest, from_uint32_tests) {
    EXPECT_FALSE(Priority::from_uint32(0).is_value());
    EXPECT_TRUE(Priority::from_uint32(1).is_value());
    EXPECT_TRUE(Priority::from_uint32(0x7FFFFFFF).is_value());
    EXPECT_TRUE(Priority::from_uint32(4294967295).is_value());
}

}
