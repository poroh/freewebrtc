//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Intrusive lists tests
//

#include <gtest/gtest.h>

#include "util/util_intrusive_list.hpp"

namespace freewebrtc::tests {

// Fixture class for the tests
class IntrusiveListTest : public ::testing::Test {
public:
    class ListItem {
    public:
        int value;
        util::IntrusiveList<ListItem>::Link link;
        ListItem(int val) : value(val), link(*this) {}
        ListItem(ListItem&& other)
            : value(other.value)
            , link(*this, std::move(other.link))
            {}
    };
    using List = util::IntrusiveList<ListItem>;

};

TEST_F(IntrusiveListTest, emptylist) {
    List list{&ListItem::link};
    EXPECT_FALSE(list.front().has_value());
    EXPECT_FALSE(list.back().has_value());
}

TEST_F(IntrusiveListTest, push_back_and_front) {
    List list{&ListItem::link};
    ListItem a(1);
    ListItem b(2);

    list.push_back(a);
    list.push_front(b);

    EXPECT_EQ(list.front()->get().value, 2);
    EXPECT_EQ(list.back()->get().value, 1);
}

TEST_F(IntrusiveListTest, item_check_in_list) {
    List list{&ListItem::link};
    ListItem a(1);
    ListItem b(2);

    list.push_back(a);
    list.push_back(b);

    EXPECT_TRUE(a.link.in_list());
    EXPECT_TRUE(b.link.in_list());
}

TEST_F(IntrusiveListTest, item_check_not_in_list_after_list_destruction) {
    auto list = std::make_unique<List>(&ListItem::link);
    ListItem a(1);
    ListItem b(2);
    ListItem c(3);

    list->push_back(a);
    list->push_back(b);
    list->push_back(c);

    EXPECT_TRUE(a.link.in_list());
    EXPECT_TRUE(b.link.in_list());
    EXPECT_TRUE(c.link.in_list());
    list.reset();
    EXPECT_TRUE(!a.link.in_list());
    EXPECT_TRUE(!b.link.in_list());
    EXPECT_TRUE(!c.link.in_list());
}

TEST_F(IntrusiveListTest, item_check_remove) {
    List list{&ListItem::link};
    ListItem a(1);
    ListItem b(2);
    ListItem c(3);

    list.push_back(a);
    list.push_back(b);
    list.push_back(c);

    EXPECT_EQ(list.front()->get().value, 1);
    EXPECT_EQ(list.back()->get().value, 3);
    c.link.remove();
    EXPECT_EQ(list.front()->get().value, 1);
    EXPECT_EQ(list.back()->get().value, 2);
    b.link.remove();
    EXPECT_EQ(list.front()->get().value, 1);
    EXPECT_EQ(list.back()->get().value, 1);
    a.link.remove();
    EXPECT_TRUE(list.empty());
}

TEST_F(IntrusiveListTest, auto_remove_on_item_dtor) {
    List list{&ListItem::link};
    auto a = std::make_unique<ListItem>(1);
    auto b = std::make_unique<ListItem>(2);
    auto c = std::make_unique<ListItem>(3);

    list.push_back(*a);
    list.push_back(*b);
    list.push_back(*c);

    EXPECT_EQ(list.front()->get().value, 1);
    EXPECT_EQ(list.back()->get().value, 3);
    b.reset();
    EXPECT_EQ(list.front()->get().value, 1);
    EXPECT_EQ(list.back()->get().value, 3);
    a.reset();
    EXPECT_EQ(list.front()->get().value, 3);
    EXPECT_EQ(list.back()->get().value, 3);
    c.reset();
    EXPECT_TRUE(list.empty());
}

TEST_F(IntrusiveListTest, place_one_item_in_two_lists) {
    List list{&ListItem::link};
    List list2{&ListItem::link};
    ListItem a(1);

    list.push_back(a);
    EXPECT_EQ(list.front()->get().value, 1);

    list2.push_back(a);
    EXPECT_TRUE(list.empty());
    EXPECT_FALSE(list2.empty());
    EXPECT_EQ(list2.front()->get().value, 1);
}

TEST_F(IntrusiveListTest, place_one_item_twice) {
    List list{&ListItem::link};
    ListItem a(1);
    ListItem b(2);
    list.push_back(a);
    list.push_back(b);
    list.push_back(a);
    EXPECT_EQ(list.front()->get().value, 2);
    EXPECT_EQ(list.back()->get().value, 1);
}

TEST_F(IntrusiveListTest, push_after_removal) {
    List list{&ListItem::link};
    ListItem a(1);
    list.push_back(a);
    a.link.remove();
    list.push_back(a);
    EXPECT_EQ(list.front()->get().value, 1);
    EXPECT_EQ(list.back()->get().value, 1);
}


TEST_F(IntrusiveListTest, remove_first_and_last_items) {
    List list{&ListItem::link};
    ListItem a(1), b(2), c(3);
    list.push_back(a);
    list.push_back(b);
    list.push_back(c);
    a.link.remove();
    c.link.remove();
    EXPECT_EQ(&list.front()->get(), &b);
    EXPECT_EQ(&list.back()->get(), &b);
}

TEST_F(IntrusiveListTest, pop_front) {
    List list{&ListItem::link};
    ListItem a(1);
    ListItem b(2);

    list.push_back(a);
    list.push_front(b);
    list.pop_front();

    EXPECT_EQ(list.front()->get().value, 1);
}

TEST_F(IntrusiveListTest, pop_back) {
    List list{&ListItem::link};
    ListItem a(1);
    ListItem b(2);
    ListItem c(3);

    list.push_back(a);
    list.push_back(b);
    list.push_back(c);
    EXPECT_EQ(list.back()->get().value, 3);
    list.pop_back();
    EXPECT_EQ(list.back()->get().value, 2);
    list.pop_back();
    EXPECT_EQ(list.back()->get().value, 1);
    list.pop_back();
    EXPECT_TRUE(list.empty());
}

TEST_F(IntrusiveListTest, empty) {
    List list{&ListItem::link};
    EXPECT_TRUE(list.empty());
    ListItem a(1);
    list.push_back(a);
    EXPECT_FALSE(list.empty());
}

TEST_F(IntrusiveListTest, link_operations) {
    List list{&ListItem::link};
    ListItem a(1);

    list.push_back(a);
    EXPECT_EQ(a.link.get().value, 1);

    a.link.remove();
    EXPECT_TRUE(list.empty());
}

TEST_F(IntrusiveListTest, list_item_move_semantics) {
    List list{&ListItem::link};
    ListItem a(1);
    list.push_back(a);
    ListItem b(std::move(a));

    EXPECT_EQ(list.front()->get().value, 1);
    EXPECT_EQ(&list.front()->get(), &b);
}

TEST_F(IntrusiveListTest, list_move_semantics) {
    List list{&ListItem::link};
    ListItem a(1);
    ListItem b(2);
    list.push_back(a);
    list.push_back(b);

    List list2(std::move(list));
    EXPECT_EQ(list2.front()->get().value, 1);
    EXPECT_EQ(list2.back()->get().value, 2);
    EXPECT_TRUE(list.empty());
}

TEST_F(IntrusiveListTest, list_move_semantics_assign_operator) {
    List list{&ListItem::link};
    List list2{&ListItem::link};
    ListItem a(1);
    ListItem b(2);
    list.push_back(a);
    list.push_back(b);

    list2 = std::move(list);
    EXPECT_EQ(list2.front()->get().value, 1);
    EXPECT_EQ(list2.back()->get().value, 2);
    EXPECT_TRUE(list.empty());
}

TEST_F(IntrusiveListTest, self_assignment) {
    List list{&ListItem::link};
    ListItem a(1), b(2);
    list.push_back(a);
    list.push_back(b);
    List& list2 = list;
    list = std::move(list2);
    EXPECT_EQ(list.front()->get().value, 1);
    EXPECT_EQ(list.back()->get().value, 2);
}

}
