//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE candidate foundation tests
//

#include <gtest/gtest.h>
#include "ice/candidate/ice_candidate_foundation.hpp"

namespace freewebrtc::tests {

class IceCandidateFoundationTest : public ::testing::Test {
public:
    using Foundation = ice::candidate::Foundation;
};

TEST_F(IceCandidateFoundationTest, from_string_tests) {
    // All characters
    EXPECT_TRUE(Foundation::from_string("ABCXYZabcxyz059+/").is_ok());
    EXPECT_FALSE(Foundation::from_string("@").is_ok()); // 'A'-1
    EXPECT_FALSE(Foundation::from_string("[").is_ok()); // 'Z'+1
    EXPECT_FALSE(Foundation::from_string("@").is_ok()); // 'A'-1
    EXPECT_FALSE(Foundation::from_string("`").is_ok()); // 'a'-1
    EXPECT_FALSE(Foundation::from_string("{").is_ok()); // 'z'+1
    EXPECT_FALSE(Foundation::from_string(".").is_ok()); // '/'-1 ('0'-1 == '/')
    EXPECT_FALSE(Foundation::from_string(":").is_ok()); // '9'+1
    // Max length
    EXPECT_TRUE(Foundation::from_string("01234567890123456789012345678901").is_ok());
    EXPECT_FALSE(Foundation::from_string("012345678901234567890123456789012").is_ok());
    // Min length
    EXPECT_TRUE(Foundation::from_string("0").is_ok());
    EXPECT_FALSE(Foundation::from_string("").is_ok());
}

}
