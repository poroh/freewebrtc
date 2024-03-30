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
    using LocalPreference = ice::candidate::LocalPreference;
    using ComponentPreference = ice::candidate::ComponentPreference;
    using ComponentId = ice::candidate::ComponentId;
};

TEST_F(IceCandidatePreferenceTest, recommended_type_preferences) {
    // See:
    // RFC5245:
    // 4.1.2.2.  Guidelines for Choosing Type and Local Preferences
    //
    // The RECOMMENDED values are 126 for host candidates, 100
    // for server reflexive candidates, 110 for peer reflexive candidates,
    // and 0 for relayed candidates.
    EXPECT_EQ(126, TypePreference::recommended_for(Type::host()).value());
    EXPECT_EQ(100, TypePreference::recommended_for(Type::server_reflexive()).value());
    EXPECT_EQ(110, TypePreference::recommended_for(Type::peer_reflexive()).value());
    EXPECT_EQ(0,   TypePreference::recommended_for(Type::relayed()).value());
}

TEST_F(IceCandidatePreferenceTest, type_preferences_from_unsigned) {
    // See:
    // RFC5245:
    // 4.1.2.2.  Guidelines for Choosing Type and Local Preferences
    //
    // The type preference MUST be an integer from 0 to 126 inclusive
    EXPECT_TRUE(TypePreference::from_unsigned(0).is_ok());
    EXPECT_TRUE(TypePreference::from_unsigned(64).is_ok());
    EXPECT_TRUE(TypePreference::from_unsigned(126).is_ok());
    EXPECT_TRUE(TypePreference::from_unsigned(127).is_err());
}

TEST_F(IceCandidatePreferenceTest, local_preferences_from_unsigned) {
    // See:
    // RFC5245:
    // 4.1.2.2.  Guidelines for Choosing Type and Local Preferences
    //
    // The local preference MUST be an integer from 0 to 65535 inclusive.
    EXPECT_TRUE(LocalPreference::from_unsigned(0).is_ok());
    EXPECT_TRUE(LocalPreference::from_unsigned(32768).is_ok());
    EXPECT_TRUE(LocalPreference::from_unsigned(65535).is_ok());
    EXPECT_TRUE(LocalPreference::from_unsigned(65546).is_err());
}

TEST_F(IceCandidatePreferenceTest, component_preferences_from_unsigned) {
    // See:
    // RFC5245:
    // 4.1.2.2.  Guidelines for Choosing Type and Local Preferences
    ASSERT_TRUE(ComponentPreference::from_unsigned(0).is_ok());
    ASSERT_TRUE(ComponentPreference::from_unsigned(128).is_ok());
    ASSERT_TRUE(ComponentPreference::from_unsigned(255).is_ok());
    ASSERT_TRUE(ComponentPreference::from_unsigned(256).is_err());
}

TEST_F(IceCandidatePreferenceTest, component_preferences_recommended_for_component) {
    // See:
    // RFC5245:
    // 4.1.2.2.  Guidelines for Choosing Type and Local Preferences
    EXPECT_EQ(ComponentPreference::recommended_for(ComponentId::from_unsigned(1).unwrap()).value(), 255);
    EXPECT_EQ(ComponentPreference::recommended_for(ComponentId::from_unsigned(5).unwrap()).value(), 251);
    EXPECT_EQ(ComponentPreference::recommended_for(ComponentId::from_unsigned(255).unwrap()).value(), 1);
}

TEST_F(IceCandidatePreferenceTest, preferences_to_priority) {
    const auto pref = ice::candidate::Preference {
        .type = TypePreference::recommended_for(Type::host()),
        .local = LocalPreference::from_unsigned(1).unwrap(),
        .component = ComponentPreference::recommended_for(ComponentId::from_unsigned(1).unwrap())
    };
    const auto prio = ice::candidate::Priority::from_uint32((126 << 24) + (1 << 8) + (256 - 1)).unwrap();
    ASSERT_TRUE(pref.to_priority().is_ok());
    EXPECT_EQ(prio, pref.to_priority().unwrap());
}


}

