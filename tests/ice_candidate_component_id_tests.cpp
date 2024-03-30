//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE candidate foundation tests
//

#include <gtest/gtest.h>
#include "ice/candidate/ice_candidate_component_id.hpp"

namespace freewebrtc::tests {

class IceCandidateComponentIdTest : public ::testing::Test {
public:
    using ComponentId = ice::candidate::ComponentId;
};

TEST_F(IceCandidateComponentIdTest, from_string_tests) {
    EXPECT_TRUE(ComponentId::from_string("0").is_ok());
    EXPECT_TRUE(ComponentId::from_string("").is_err());
    EXPECT_TRUE(ComponentId::from_string("123").is_ok());
    EXPECT_TRUE(ComponentId::from_string("256").is_ok());
    EXPECT_TRUE(ComponentId::from_string("1234").is_err());

    EXPECT_TRUE(ComponentId::from_string("90").is_ok());
    EXPECT_EQ(ComponentId::from_string("1").unwrap(), ComponentId::from_unsigned(1).unwrap());
    EXPECT_EQ(ComponentId::from_string("123").unwrap(), ComponentId::from_unsigned(123).unwrap());
}

}
