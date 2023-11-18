//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Return Value Tests
//

#include <gtest/gtest.h>
#include <memory>
#include <queue>
#include "util/util_return_value.hpp"

namespace freewebrtc::test {

class UtilReturnValueTest : public ::testing::Test {
};

// ================================================================================
// Construction / destruction / assignments etc.

TEST_F(UtilReturnValueTest, check_value) {
    ReturnValue<int> rv(1);
    ASSERT_TRUE(rv.is_value());
    EXPECT_EQ(rv.assert_value(), 1);
    ASSERT_TRUE(rv.maybe_value().has_value());
    EXPECT_EQ(rv.maybe_value().value(), 1);
    EXPECT_FALSE(rv.is_error());
}

TEST_F(UtilReturnValueTest, check_error) {
    auto ec = std::make_error_code(std::errc::invalid_argument);
    ReturnValue<int> rv(ec);
    ASSERT_TRUE(rv.is_error());
    EXPECT_EQ(rv.assert_error(), ec);
    EXPECT_FALSE(rv.is_value());
    EXPECT_FALSE(rv.maybe_value().has_value());
}

TEST_F(UtilReturnValueTest, rvalue_constructor) {
    using T = std::unique_ptr<int>;
     // cannot be constructed without rvalue constructor
    ReturnValue<T> rv(std::make_unique<int>(1));
    ASSERT_TRUE(rv.is_value());
    ASSERT_EQ(*rv.assert_value().get(), 1);
}

TEST_F(UtilReturnValueTest, copy_constructor_with_value) {
    ReturnValue<int> original(42);
    ReturnValue<int> copy = original;
    ASSERT_TRUE(copy.is_value());
    EXPECT_EQ(copy.assert_value(), 42);
}

TEST_F(UtilReturnValueTest, move_constructor_with_value) {
    using T = std::unique_ptr<int>;
    ReturnValue<T> original(std::make_unique<int>(42));
    ReturnValue<T> moved(std::move(original));
    ASSERT_TRUE(moved.is_value());
    EXPECT_EQ(*moved.assert_value().get(), 42);
    ASSERT_TRUE(original.is_value());
    EXPECT_EQ(original.assert_value().get(), nullptr);
}

TEST_F(UtilReturnValueTest, copy_assignment_with_value) {
    ReturnValue<int> original(42);
    ReturnValue<int> copy(0);
    copy = original;
    ASSERT_TRUE(copy.is_value());
    EXPECT_EQ(copy.assert_value(), 42);
}

TEST_F(UtilReturnValueTest, move_assignment_with_value) {
    using T = std::unique_ptr<int>;
    ReturnValue<T> original(std::make_unique<int>(42));
    ReturnValue<T> moved(std::make_unique<int>(0));
    moved = std::move(original); // Move assignment
    ASSERT_TRUE(moved.is_value());
    EXPECT_EQ(*moved.assert_value().get(), 42);
    ASSERT_TRUE(original.is_value());
    EXPECT_EQ(original.assert_value().get(), nullptr);
}

// ================================================================================
// fmap

TEST_F(UtilReturnValueTest, fmap_value_reference) {
     // cannot be constructed without rvalue constructor
    ReturnValue<int> int_rv(1);
    auto string_rv = int_rv.fmap([](auto i) { return std::to_string(i); });
    ASSERT_TRUE(string_rv.is_value());
    EXPECT_EQ(string_rv.assert_value(), "1");
}

TEST_F(UtilReturnValueTest, fmap_error) {
    auto ec = std::make_error_code(std::errc::invalid_argument);
    ReturnValue<int> int_rv(ec);
    auto string_rv = int_rv.fmap([](auto i) { return std::to_string(i); });
    ASSERT_TRUE(string_rv.is_error());
    EXPECT_EQ(string_rv.assert_error(), ec);
}

TEST_F(UtilReturnValueTest, fmap_rvalue) {
    using T = std::unique_ptr<int>;
    ReturnValue<T> intptr_rv(std::make_unique<int>(1));
    // It would compile without fmap for r-value
    auto string_rv = std::move(intptr_rv)
        .fmap([](auto i) { return std::to_string(*i.get()); });
    ASSERT_TRUE(string_rv.is_value());
    EXPECT_EQ(string_rv.assert_value(), "1");
}

TEST_F(UtilReturnValueTest, fmap_reference) {
    using T = std::unique_ptr<int>;
    ReturnValue<T> intptr_rv(std::make_unique<int>(1));
    // It would compile without fmap for reference
    auto string_rv = intptr_rv
        .fmap([](T& i) { i.reset(); return "1"; });
    ASSERT_TRUE(string_rv.is_value());
    EXPECT_EQ(string_rv.assert_value(), "1");
}

TEST_F(UtilReturnValueTest, fmap_const_reference) {
    using T = std::unique_ptr<int>;
    const ReturnValue<T> intptr_rv(std::make_unique<int>(1));
    // It would compile without fmap for const reference
    auto string_rv = intptr_rv
        .fmap([](const T& i) { return std::to_string(*i.get()); });
    ASSERT_TRUE(string_rv.is_value());
    EXPECT_EQ(string_rv.assert_value(), "1");
}

TEST_F(UtilReturnValueTest, fmap_error_propagation) {
    auto ec = std::make_error_code(std::errc::invalid_argument);
    ReturnValue<int> rv(ec);
    auto chained_rv = rv
        .fmap([](int value) { return value + 1; })
        .fmap([](int value) { return value * 2; });
    EXPECT_TRUE(chained_rv.is_error());
    EXPECT_EQ(chained_rv.assert_error(), ec);
}

TEST_F(UtilReturnValueTest, fmap_nested_return_value_unwrapping) {
    ReturnValue<int> nested_rv(1);
    auto unwrapped_rv = nested_rv
        .fmap([](auto& i) { return ReturnValue<int>(i * 2); });
    ASSERT_TRUE(unwrapped_rv.is_value());
    EXPECT_EQ(unwrapped_rv.assert_value(), 2);
}

TEST_F(UtilReturnValueTest, fmap_nested_return_value_unwrapping_error) {
    auto ec = std::make_error_code(std::errc::invalid_argument);
    ReturnValue<int> nested_rv(1);
    auto unwrapped_rv = nested_rv
        .fmap([&](auto&) { return ReturnValue<int>(ec); });
    EXPECT_TRUE(unwrapped_rv.is_error());
    EXPECT_EQ(unwrapped_rv.assert_error(), ec);
}

// ================================================================================
// combine

TEST_F(UtilReturnValueTest, combine_two_values_success) {
    ReturnValue<int> rv1(10);
    ReturnValue<int> rv2(20);
    auto combined_rv = combine([](int a, int b) { return a + b; }, std::move(rv1), std::move(rv2));
    ASSERT_TRUE(combined_rv.is_value());
    EXPECT_EQ(combined_rv.assert_value(), 30);
}

TEST_F(UtilReturnValueTest, combine_value_with_error) {
    ReturnValue<int> rv1(10);
    auto ec = std::make_error_code(std::errc::invalid_argument);
    ReturnValue<int> rv2(ec);
    auto combined_rv = combine([](int a, int b) { return a + b; }, std::move(rv1), std::move(rv2));
    ASSERT_TRUE(combined_rv.is_error());
    EXPECT_EQ(combined_rv.assert_error(), ec);
}

TEST_F(UtilReturnValueTest, combine_multiple_values_success) {
    ReturnValue<int> rv1(10);
    ReturnValue<int> rv2(20);
    ReturnValue<int> rv3(30);
    auto combined_rv = combine([](int a, int b, int c) { return a + b + c; }, std::move(rv1), std::move(rv2), std::move(rv3));
    ASSERT_TRUE(combined_rv.is_value());
    EXPECT_EQ(combined_rv.assert_value(), 60);
}

TEST_F(UtilReturnValueTest, combine_multiple_values_one_error) {
    ReturnValue<int> rv1(10);
    ReturnValue<int> rv2(20);
    auto ec = std::make_error_code(std::errc::invalid_argument);
    ReturnValue<int> rv3(ec);
    auto combined_rv = combine([](int a, int b, int c) { return a + b + c; }, std::move(rv1), std::move(rv2), std::move(rv3));
    ASSERT_TRUE(combined_rv.is_error());
    EXPECT_EQ(combined_rv.assert_error(), ec);
}

TEST_F(UtilReturnValueTest, combine_moved_rvalues) {
    using T = std::unique_ptr<int>;
    ReturnValue<T> rv1(std::make_unique<int>(10));
    ReturnValue<T> rv2(std::make_unique<int>(20));
    auto combined_rv = combine([](T&& p1, T&& p2) {
        std::vector<T> vec;
        vec.emplace_back(std::move(p1));
        vec.emplace_back(std::move(p2));
        return vec;
    }, std::move(rv1), std::move(rv2));

    ASSERT_TRUE(combined_rv.is_value());
    auto& vec = combined_rv.assert_value();

    ASSERT_EQ(vec.size(), 2u); // Ensure there are two elements in the vector
    EXPECT_EQ(*vec[0], 10);    // Verify the first unique_ptr's value
    EXPECT_EQ(*vec[1], 20);

    ASSERT_TRUE(rv1.is_value());
    ASSERT_TRUE(rv2.is_value());
    EXPECT_EQ(rv1.assert_value().get(), nullptr);
    EXPECT_EQ(rv2.assert_value().get(), nullptr);
}

TEST_F(UtilReturnValueTest, combine_references) {
    using T = std::unique_ptr<int>;
    ReturnValue<T> rv1(std::make_unique<int>(10));
    ReturnValue<T> rv2(std::make_unique<int>(20));
    auto combined_rv = combine([](T& p1, T& p2) {
        std::vector<T> vec;
        vec.emplace_back(std::move(p1));
        vec.emplace_back(std::move(p2));
        return vec;
    }, rv1, rv2);

    ASSERT_TRUE(combined_rv.is_value());
    auto& vec = combined_rv.assert_value();

    ASSERT_EQ(vec.size(), 2u); // Ensure there are two elements in the vector
    EXPECT_EQ(*vec[0], 10);    // Verify the first unique_ptr's value
    EXPECT_EQ(*vec[1], 20);

    ASSERT_TRUE(rv1.is_value());
    ASSERT_TRUE(rv2.is_value());
    EXPECT_EQ(rv1.assert_value().get(), nullptr);
    EXPECT_EQ(rv2.assert_value().get(), nullptr);
}

TEST_F(UtilReturnValueTest, combine_const_references) {
    using T = std::unique_ptr<int>;
    const ReturnValue<T> rv1(std::make_unique<int>(10));
    const ReturnValue<T> rv2(std::make_unique<int>(20));
    auto combined_rv = combine([](const T& p1, const T& p2) {
        return *p1.get() + *p2.get();
    }, rv1, rv2);

    ASSERT_TRUE(combined_rv.is_value());
    EXPECT_EQ(combined_rv.assert_value(), 30);
}

}

