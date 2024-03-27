//
// Copyright (c) 2024 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE candidate preference tests
//

#include <gtest/gtest.h>
#include "ice/candidate/ice_candidate_preference.hpp"

namespace freewebrtc::tests {

class IceCandidatePreferenceTest : public ::testing::Test {
public:
    using Type = ice::candidate::Type;
    using TypePreference = ice::candidate::TypePreference;
};

TEST_F(IceCandidatePreferenceTest, recommended_type_preferences) {
    // See:
    // RFC5245:
    // 4.1.2.2.  Guidelines for Choosing Type and Local Preferences
    //
    // The RECOMMENDED values are 126 for host candidates, 100
    // for server reflexive candidates, 110 for peer reflexive candidates,
    // and 0 for relayed candidates. 
    ASSERT_EQ(126, TypePreference::recommended_for(Type::host()).value());
    ASSERT_EQ(100, TypePreference::recommended_for(Type::server_reflexive()).value());
    ASSERT_EQ(110, TypePreference::recommended_for(Type::peer_reflexive()).value());
    ASSERT_EQ(0,   TypePreference::recommended_for(Type::relayed()).value());
}

}

