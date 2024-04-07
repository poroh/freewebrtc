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
#include "util/util_result.hpp"

namespace freewebrtc::test {

class UtilResultTest : public ::testing::Test {
public:
    using RVI = Result<int>;
};

// ================================================================================
// Construction / destruction / assignments etc.

TEST_F(UtilResultTest, check_value) {
    Result<int> rv(1);
    ASSERT_TRUE(rv.is_ok());
    EXPECT_EQ(rv.unwrap(), 1);
    EXPECT_FALSE(rv.is_err());
}

TEST_F(UtilResultTest, check_error) {
    auto ec = std::make_error_code(std::errc::invalid_argument);
    Result<int> rv(ec);
    ASSERT_TRUE(rv.is_err());
    EXPECT_EQ(rv.unwrap_err(), ec);
    EXPECT_FALSE(rv.is_ok());
}

TEST_F(UtilResultTest, rvalue_constructor) {
    using T = std::unique_ptr<int>;
     // cannot be constructed without rvalue constructor
    Result<T> rv(std::make_unique<int>(1));
    ASSERT_TRUE(rv.is_ok());
    ASSERT_EQ(*rv.unwrap().get(), 1);
}

TEST_F(UtilResultTest, copy_constructor_with_value) {
    Result<int> original(42);
    Result<int> copy = original;
    ASSERT_TRUE(copy.is_ok());
    EXPECT_EQ(copy.unwrap(), 42);
}

TEST_F(UtilResultTest, move_constructor_with_value) {
    using T = std::unique_ptr<int>;
    Result<T> original(std::make_unique<int>(42));
    Result<T> moved(std::move(original));
    ASSERT_TRUE(moved.is_ok());
    EXPECT_EQ(*moved.unwrap().get(), 42);
    ASSERT_TRUE(original.is_ok());
    EXPECT_EQ(original.unwrap().get(), nullptr);
}

TEST_F(UtilResultTest, copy_assignment_with_value) {
    Result<int> original(42);
    Result<int> copy(0);
    copy = original;
    ASSERT_TRUE(copy.is_ok());
    EXPECT_EQ(copy.unwrap(), 42);
}

TEST_F(UtilResultTest, move_assignment_with_value) {
    using T = std::unique_ptr<int>;
    Result<T> original(std::make_unique<int>(42));
    Result<T> moved(std::make_unique<int>(0));
    moved = std::move(original); // Move assignment
    ASSERT_TRUE(moved.is_ok());
    EXPECT_EQ(*moved.unwrap().get(), 42);
    ASSERT_TRUE(original.is_ok());
    EXPECT_EQ(original.unwrap().get(), nullptr);
}

// ================================================================================
// fmap

TEST_F(UtilResultTest, fmap_value_reference) {
     // cannot be constructed without rvalue constructor
    Result<int> int_rv(1);
    auto string_rv = int_rv.fmap([](auto i) { return std::to_string(i); });
    ASSERT_TRUE(string_rv.is_ok());
    EXPECT_EQ(string_rv.unwrap(), "1");
}

TEST_F(UtilResultTest, fmap_error) {
    auto ec = std::make_error_code(std::errc::invalid_argument);
    Result<int> int_rv(ec);
    auto string_rv = int_rv.fmap([](auto i) { return std::to_string(i); });
    ASSERT_TRUE(string_rv.is_err());
    EXPECT_EQ(string_rv.unwrap_err(), ec);
}

TEST_F(UtilResultTest, fmap_rvalue) {
    using T = std::unique_ptr<int>;
    Result<T> intptr_rv(std::make_unique<int>(1));
    // It would compile without fmap for r-value
    auto string_rv = std::move(intptr_rv)
        .fmap([](auto i) { return std::to_string(*i.get()); });
    ASSERT_TRUE(string_rv.is_ok());
    EXPECT_EQ(string_rv.unwrap(), "1");
}

TEST_F(UtilResultTest, fmap_reference) {
    using T = std::unique_ptr<int>;
    Result<T> intptr_rv(std::make_unique<int>(1));
    // It would compile without fmap for reference
    auto string_rv = intptr_rv
        .fmap([](T& i) { i.reset(); return "1"; });
    ASSERT_TRUE(string_rv.is_ok());
    EXPECT_EQ(string_rv.unwrap(), "1");
}

TEST_F(UtilResultTest, fmap_const_reference) {
    using T = std::unique_ptr<int>;
    const Result<T> intptr_rv(std::make_unique<int>(1));
    // It would compile without fmap for const reference
    auto string_rv = intptr_rv
        .fmap([](const T& i) { return std::to_string(*i.get()); });
    ASSERT_TRUE(string_rv.is_ok());
    EXPECT_EQ(string_rv.unwrap(), "1");
}

TEST_F(UtilResultTest, fmap_error_propagation) {
    auto ec = std::make_error_code(std::errc::invalid_argument);
    Result<int> rv(ec);
    auto chained_rv = rv
        .fmap([](int value) { return value + 1; })
        .fmap([](int value) { return value * 2; });
    ASSERT_TRUE(chained_rv.is_err());
    EXPECT_EQ(chained_rv.unwrap_err(), ec);
}

// ================================================================================
// bind

TEST_F(UtilResultTest, bind_return_value_unwrapping) {
    Result<int> nested_rv(1);
    auto unwrapped_rv = nested_rv
        .bind([](auto& i) { return Result<int>(i * 2); });
    ASSERT_TRUE(unwrapped_rv.is_ok());
    EXPECT_EQ(unwrapped_rv.unwrap(), 2);
}

TEST_F(UtilResultTest, bind_return_value_unwrapping_error) {
    auto ec = std::make_error_code(std::errc::invalid_argument);
    Result<int> nested_rv(1);
    auto unwrapped_rv = nested_rv
        .bind([&](auto&) { return Result<int>(ec); });
    ASSERT_TRUE(unwrapped_rv.is_err());
    EXPECT_EQ(unwrapped_rv.unwrap_err(), ec);
}

// ================================================================================
// combine

TEST_F(UtilResultTest, combine_two_values_success) {
    using RVI = Result<int>;
    Result<int> rv1(10);
    Result<int> rv2(20);
    auto combined_rv = combine([](int a, int b) -> RVI { return a + b; }, std::move(rv1), std::move(rv2));
    ASSERT_TRUE(combined_rv.is_ok());
    EXPECT_EQ(combined_rv.unwrap(), 30);
}

TEST_F(UtilResultTest, combine_value_with_error) {
    Result<int> rv1(10);
    auto ec = std::make_error_code(std::errc::invalid_argument);
    Result<int> rv2(ec);
    auto combined_rv = combine([](int a, int b) -> RVI { return a + b; }, std::move(rv1), std::move(rv2));
    ASSERT_TRUE(combined_rv.is_err());
    EXPECT_EQ(combined_rv.unwrap_err(), ec);
}

TEST_F(UtilResultTest, combine_multiple_values_success) {
    Result<int> rv1(10);
    Result<int> rv2(20);
    Result<int> rv3(30);
    auto combined_rv = combine([](int a, int b, int c) -> RVI { return a + b + c; }, std::move(rv1), std::move(rv2), std::move(rv3));
    ASSERT_TRUE(combined_rv.is_ok());
    EXPECT_EQ(combined_rv.unwrap(), 60);
}

TEST_F(UtilResultTest, combine_multiple_values_one_error) {
    Result<int> rv1(10);
    Result<int> rv2(20);
    auto ec = std::make_error_code(std::errc::invalid_argument);
    Result<int> rv3(ec);
    auto combined_rv = combine([](int a, int b, int c) -> RVI { return a + b + c; }, std::move(rv1), std::move(rv2), std::move(rv3));
    ASSERT_TRUE(combined_rv.is_err());
    EXPECT_EQ(combined_rv.unwrap_err(), ec);
}

TEST_F(UtilResultTest, combine_moved_rvalues) {
    using T = std::unique_ptr<int>;
    Result<T> rv1(std::make_unique<int>(10));
    Result<T> rv2(std::make_unique<int>(20));
    auto combined_rv = combine([](T&& p1, T&& p2) -> Result<std::vector<T>> {
        std::vector<T> vec;
        vec.emplace_back(std::move(p1));
        vec.emplace_back(std::move(p2));
        return vec;
    }, std::move(rv1), std::move(rv2));

    ASSERT_TRUE(combined_rv.is_ok());
    auto& vec = combined_rv.unwrap();

    ASSERT_EQ(vec.size(), 2u); // Ensure there are two elements in the vector
    EXPECT_EQ(*vec[0], 10);    // Verify the first unique_ptr's value
    EXPECT_EQ(*vec[1], 20);

    ASSERT_TRUE(rv1.is_ok());
    ASSERT_TRUE(rv2.is_ok());
    EXPECT_EQ(rv1.unwrap().get(), nullptr);
    EXPECT_EQ(rv2.unwrap().get(), nullptr);
}

TEST_F(UtilResultTest, combine_references) {
    using T = std::unique_ptr<int>;
    Result<T> rv1(std::make_unique<int>(10));
    Result<T> rv2(std::make_unique<int>(20));
    auto combined_rv = combine([](T& p1, T& p2) -> Result<std::vector<T>> {
        std::vector<T> vec;
        vec.emplace_back(std::move(p1));
        vec.emplace_back(std::move(p2));
        return vec;
    }, rv1, rv2);

    ASSERT_TRUE(combined_rv.is_ok());
    auto& vec = combined_rv.unwrap();

    ASSERT_EQ(vec.size(), 2u); // Ensure there are two elements in the vector
    EXPECT_EQ(*vec[0], 10);    // Verify the first unique_ptr's value
    EXPECT_EQ(*vec[1], 20);

    ASSERT_TRUE(rv1.is_ok());
    ASSERT_TRUE(rv2.is_ok());
    EXPECT_EQ(rv1.unwrap().get(), nullptr);
    EXPECT_EQ(rv2.unwrap().get(), nullptr);
}

TEST_F(UtilResultTest, combine_const_references) {
    using T = std::unique_ptr<int>;
    const Result<T> rv1(std::make_unique<int>(10));
    const Result<T> rv2(std::make_unique<int>(20));
    auto combined_rv = combine([](const T& p1, const T& p2) -> RVI {
        return *p1.get() + *p2.get();
    }, rv1, rv2);

    ASSERT_TRUE(combined_rv.is_ok());
    EXPECT_EQ(combined_rv.unwrap(), 30);
}

}

