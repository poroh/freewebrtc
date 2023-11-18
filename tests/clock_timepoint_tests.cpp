//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Timepoint tests
//

#include "clock/clock_timepoint.hpp"
#include <gtest/gtest.h>
#include <limits>

namespace freewebrtc::test {

class TimepointTest : public ::testing::Test {
protected:
    clock::Timepoint epoch = clock::Timepoint::epoch();
    static constexpr auto min = std::numeric_limits<clock::Timepoint::Duration::rep>::min();
    static constexpr auto max = std::numeric_limits<clock::Timepoint::Duration::rep>::max();
};

TEST_F(TimepointTest, epoch_timepoint) {
    EXPECT_EQ(epoch.is_after(clock::Timepoint::epoch()), false);
    EXPECT_EQ(epoch.is_before(clock::Timepoint::epoch()), false);
}

TEST_F(TimepointTest, advance_from) {
    std::chrono::milliseconds duration(100);
    auto future = epoch.advance_from(duration);
    EXPECT_EQ(duration.count(), 0);
    EXPECT_TRUE(future.is_after(epoch));
}

TEST_F(TimepointTest, advance_from_nanoseconds) {
    std::chrono::nanoseconds duration(100);
    auto notincreased = epoch.advance_from(duration);
    EXPECT_EQ(notincreased, epoch);
    duration += std::chrono::nanoseconds(900);
    auto increased = epoch.advance_from(duration);
    EXPECT_EQ(duration, std::chrono::nanoseconds(0));
    EXPECT_EQ((increased - epoch).count(), 1);
}

TEST_F(TimepointTest, subtraction_operator) {
    const std::chrono::milliseconds ms100(100);
    auto duration = ms100;
    auto future = epoch.advance_from(duration);
    auto v = future - epoch;
    EXPECT_EQ(v, ms100);
}

TEST_F(TimepointTest, is_after) {
    std::chrono::milliseconds duration(100);
    auto future = epoch.advance_from(duration);

    EXPECT_TRUE(future.is_after(epoch));
    EXPECT_FALSE(epoch.is_after(future));
}

TEST_F(TimepointTest, is_before) {
    std::chrono::milliseconds duration(100);
    auto future = epoch.advance_from(duration);

    EXPECT_TRUE(epoch.is_before(future));
    EXPECT_FALSE(future.is_before(epoch));
}

TEST_F(TimepointTest, equality_operator) {
    auto another = clock::Timepoint::epoch();
    EXPECT_EQ(epoch, another);
}

TEST_F(TimepointTest, large_duration_advancement) {
    std::chrono::hours large_duration(24 * 365 * 10); // 10 years
    auto future_timepoint = epoch.advance_from(large_duration);
    EXPECT_TRUE(future_timepoint.is_after(epoch));
}

TEST_F(TimepointTest, backward_duration_advancement) {
    std::chrono::milliseconds negative_duration(-100);
    auto past_timepoint = epoch.advance_from(negative_duration);
    EXPECT_TRUE(past_timepoint.is_before(epoch));
}

TEST_F(TimepointTest, boundary_conditions) {
    std::chrono::microseconds max_duration(max);
    auto max_timepoint = epoch.advance_from(max_duration);
    EXPECT_TRUE(max_timepoint.is_after(epoch));

    std::chrono::microseconds min_duration(min);
    auto min_timepoint = epoch.advance_from(min_duration);
    EXPECT_TRUE(min_timepoint.is_before(epoch));
}

TEST_F(TimepointTest, precision_test) {
    std::chrono::nanoseconds one_nano(1);
    epoch.advance_from(one_nano);
    EXPECT_EQ(one_nano.count(), 1); // since internal precision is microseconds
}

// Test for subtraction edge cases
TEST_F(TimepointTest, subtraction_edge_cases) {
    const std::chrono::hours large_duration(24 * 365 * 10); // 10 years
    auto large_copy = large_duration;
    auto future_timepoint = epoch.advance_from(large_copy);
    auto calculated_duration = future_timepoint - epoch;
    EXPECT_EQ(calculated_duration, large_duration);
}

// Test for equality after advancement
TEST_F(TimepointTest, equality_after_advancement) {
    const std::chrono::milliseconds duration(100);
    auto d1 = duration;
    auto timepoint1 = epoch.advance_from(d1);
    auto d2 = duration;
    auto timepoint2 = epoch.advance_from(d2);
    EXPECT_EQ(timepoint1, timepoint2);
}

// Test for ordering after advancement
TEST_F(TimepointTest, ordering_after_advancement) {
    std::chrono::milliseconds duration(100);
    auto future_timepoint = epoch.advance_from(duration);
    EXPECT_TRUE(future_timepoint.is_after(epoch));
    EXPECT_FALSE(epoch.is_after(future_timepoint));
}

// Test for overflow/underflow scenarios
TEST_F(TimepointTest, overflow_underflow_scenarios) {
    std::chrono::microseconds max_duration(max);
    const std::chrono::microseconds one_micro(1);
    auto one_micro_copy = one_micro;
    auto max_timepoint = epoch.advance_from(max_duration);
    auto next_timepoint = max_timepoint.advance_from(one_micro_copy);
    // two timepoints next to each other must be "after"
    EXPECT_TRUE(next_timepoint.is_after(max_timepoint));

    std::chrono::microseconds min_duration(min);
    auto min_timepoint = epoch.advance_from(min_duration);
    auto minus_one_micro = -one_micro;
    auto prev_timepoint = min_timepoint.advance_from(minus_one_micro);
    EXPECT_TRUE(prev_timepoint.is_before(min_timepoint));
}

// Test for crossing epoch
TEST_F(TimepointTest, crossing_epoch) {
    std::chrono::milliseconds pre_epoch_duration(-100);
    auto pre_epoch_timepoint = epoch.advance_from(pre_epoch_duration);
    std::chrono::milliseconds post_epoch_duration(200);
    auto post_epoch_timepoint = pre_epoch_timepoint.advance_from(post_epoch_duration);
    EXPECT_TRUE(post_epoch_timepoint.is_after(epoch));
}

// Test for duration reduction accuracy
TEST_F(TimepointTest, duration_reduction_accuracy) {
    std::chrono::nanoseconds duration(1001);
    epoch.advance_from(duration);
    EXPECT_EQ(duration.count(), 1); // Expect 1 nanosecond remaining
}

}
