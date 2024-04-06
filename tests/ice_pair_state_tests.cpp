//
// Copyright (c) 2024 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE candidate pair state tests
//

#include <gtest/gtest.h>
#include "ice/pair/ice_pair_state.hpp"

namespace freewebrtc::tests {

class IcePairStateTest : public ::testing::Test {
public:
    using State = ice::pair::State;
    using Event = ice::pair::Event;
};

TEST_F(IcePairStateTest, valid_transitions) {
    // See RFC8445
    // 6.1.2.6.  Computing Candidate Pair States
    ASSERT_TRUE(State::frozen().transition(Event::unfreeze()).is_ok());
    ASSERT_TRUE(State::waiting().transition(Event::perform()).is_ok());
    ASSERT_TRUE(State::in_progress().transition(Event::failure()).is_ok());
    ASSERT_TRUE(State::in_progress().transition(Event::success()).is_ok());

    EXPECT_EQ(State::frozen().transition(Event::unfreeze()).unwrap(), State::waiting());
    EXPECT_EQ(State::waiting().transition(Event::perform()).unwrap(), State::in_progress());
    EXPECT_EQ(State::in_progress().transition(Event::failure()).unwrap(), State::failed());
    EXPECT_EQ(State::in_progress().transition(Event::success()).unwrap(), State::succeeded());
}

TEST_F(IcePairStateTest, invalid_transitions) {
    // See RFC8445
    // 6.1.2.6.  Computing Candidate Pair States
    EXPECT_TRUE(State::frozen().transition(Event::perform()).is_err());
    EXPECT_TRUE(State::frozen().transition(Event::success()).is_err());
    EXPECT_TRUE(State::frozen().transition(Event::failure()).is_err());

    EXPECT_TRUE(State::waiting().transition(Event::unfreeze()).is_err());
    EXPECT_TRUE(State::waiting().transition(Event::success()).is_err());
    EXPECT_TRUE(State::waiting().transition(Event::failure()).is_err());

    EXPECT_TRUE(State::in_progress().transition(Event::perform()).is_err());
    EXPECT_TRUE(State::in_progress().transition(Event::unfreeze()).is_err());
}

}
