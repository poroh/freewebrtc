//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Implementation of intrusive list
// Very similar concept to boost::intrusive_list
// Freewebrtc does not use boost so it is implemented here.
//

#pragma once

#include <cstdlib>
#include <functional>

#include "util/util_maybe.hpp"

namespace freewebrtc::util {

template<typename T>
class IntrusiveList {
public:
    using Ref = std::reference_wrapper<T>;
    using CRef = std::reference_wrapper<const T>;
    class Link;
    // Create with pointer on Link member.
    explicit IntrusiveList(Link T::* link);
    // Intrusive list cannot have copy.
    IntrusiveList(const IntrusiveList&) = delete;
    // O(1)
    IntrusiveList(IntrusiveList&&);
    // Destructor is O(N) since it needs to unlink all items in the list
    ~IntrusiveList();

    // No copy, only move semantic
    IntrusiveList& operator=(const IntrusiveList&) = delete;
    // O(1) for move
    IntrusiveList& operator=(IntrusiveList&&);

    // Push element to list. Rewrites link. O(1)
    void push_back(T&) noexcept;
    void push_front(T&) noexcept;
    // Pop element from link O(1)
    void pop_back() noexcept;
    void pop_front() noexcept;

    // Access O(1)
    Maybe<Ref> front() noexcept;
    Maybe<CRef> front() const noexcept;
    Maybe<Ref> back() noexcept;
    Maybe<CRef> back() const noexcept;

    // Emptiness O(1)
    bool empty() const noexcept;
    // Clear is O(N)
    void clear() noexcept;

private:
    void do_move(IntrusiveList&) noexcept;
    using LinkRef = std::reference_wrapper<Link>;
    Link T::* m_link_member;
    Link m_head;
    Link m_tail;
};

template<typename T>
class IntrusiveList<T>::Link {
public:
    Link(T&);
    Link(T& t, Link&&);
    // Cannot move link from one object to another.
    // Need to keep consistency with reference to object itself and
    // above constructor should be used
    Link(Link&&) = delete;
    ~Link();
    // No any types of copy
    Link& operator=(const Link&) = delete;
    Link& operator=(Link&&) = default;

    T& get() noexcept;
    const T& get() const noexcept;
    // Remove item from list (O(1))
    void remove() noexcept;
    // Check that item is in list
    bool in_list() const noexcept;

private:
    friend class IntrusiveList<T>;
    Link() = default;
    Link(const Link&) = default;
    void place_after(Link&) noexcept;
    void place_before(Link&) noexcept;
    Maybe<Ref> m_owner = None{};
    Maybe<LinkRef> m_next = None{};
    Maybe<LinkRef> m_prev = None{};
};

//
// implementation
//
template<typename T>
IntrusiveList<T>::Link::Link(T& t)
    : m_owner(t)
{}

template<typename T>
IntrusiveList<T>::Link::Link(T& t, Link&& other)
    : m_owner(t)
    , m_next(other.m_next)
    , m_prev(other.m_prev)
{
    m_prev.fmap([&](auto&& prev) {
        prev.get().m_next = LinkRef{*this};
        return Unit{};
    });
    m_next.fmap([&](auto&& next) {
        next.get().m_prev = LinkRef{*this};
        return Unit{};
    });
    other.m_next = none();
    other.m_prev = none();
}

template<typename T>
IntrusiveList<T>::Link::~Link() {
    remove();
}

template<typename T>
T& IntrusiveList<T>::Link::get() noexcept {
    if (!m_owner.is_some()) {
        std::abort();
    }
    return m_owner.unwrap();
}

template<typename T>
const T& IntrusiveList<T>::Link::get() const noexcept {
    if (!m_owner.has_value()) {
        std::abort();
    }
    return m_owner.value();
}

template<typename T>
void IntrusiveList<T>::Link::remove() noexcept {
    m_prev.fmap([&](auto&& prev) {
        prev.get().m_next = m_next;
        return Unit{};
    });
    m_next.fmap([&](auto&& next) {
        next.get().m_prev = m_prev;
        return Unit{};
    });
    m_prev = none();
    m_next = none();
}

template<typename T>
void IntrusiveList<T>::Link::place_after(Link& t) noexcept {
    remove();
    m_next = t.m_next;
    t.m_next = LinkRef{*this};
    m_prev = LinkRef{t};
    m_next.fmap([&](auto&& next) {
        next.get().m_prev = LinkRef{*this};
        return Unit{};
    });
}

template<typename T>
void IntrusiveList<T>::Link::place_before(Link& t) noexcept {
    remove();
    m_next = LinkRef{t};
    m_prev = t.m_prev;
    t.m_prev = LinkRef{*this};
    m_prev.fmap([&](auto&& prev) {
        prev.get().m_next = LinkRef{*this};
        return Unit{};
    });
}

template<typename T>
bool IntrusiveList<T>::Link::in_list() const noexcept {
    return m_next.is_some() && m_prev.is_some();
}

template<typename T>
IntrusiveList<T>::IntrusiveList(Link T::* link)
    : m_link_member(link)
{
    m_tail.place_after(m_head);
}

template<typename T>
IntrusiveList<T>::IntrusiveList(IntrusiveList&& other)
    : m_link_member(other.m_link_member)
{
    if (other.empty()) {
        m_tail.place_after(m_head);
        return;
    }
    do_move(other);
}

template<typename T>
IntrusiveList<T>::~IntrusiveList() {
    clear();
}

template<typename T>
IntrusiveList<T>& IntrusiveList<T>::operator=(IntrusiveList&& other) {
    if (&other == this) { // self assignment
        return *this;
    }
    clear();
    if (other.empty()) {
        return *this;
    }
    do_move(other);
    return *this;
}

template<typename T>
void IntrusiveList<T>::push_back(T& t) noexcept {
    (t.*m_link_member).place_before(m_tail);
}

template<typename T>
void IntrusiveList<T>::push_front(T& t) noexcept {
    (t.*m_link_member).place_after(m_head);
}

template<typename T>
void IntrusiveList<T>::pop_front() noexcept {
    if (empty()) {
        std::abort();
    }
    m_head.m_next.fmap([](auto&& next) {
        next.get().remove();
        return Unit{};
    });
}

template<typename T>
void IntrusiveList<T>::pop_back() noexcept {
    if (empty()) {
        std::abort();
    }
    m_tail.m_prev.fmap([](auto&& prev) {
        prev.get().remove();
        return Unit{};
    });
}

template<typename T>
Maybe<typename IntrusiveList<T>::Ref>
IntrusiveList<T>::front() noexcept {
    if (empty()) {
        return none();
    }
    return m_head.m_next
        .fmap([&](auto&& next) {
            return std::ref(next.get().get());
        });
}

template<typename T>
Maybe<typename IntrusiveList<T>::CRef>
IntrusiveList<T>::front() const noexcept {
    if (empty()) {
        return none();
    }
    return m_head.m_next
        .fmap([&](auto&& next) {
            return std::cref(next.get().get());
        });
}

template<typename T>
void IntrusiveList<T>::clear() noexcept {
    while (!empty()) {
        pop_front();
    }
}

template<typename T>
Maybe<typename IntrusiveList<T>::Ref>
IntrusiveList<T>::back() noexcept {
    if (empty()) {
        return none();
    }
    return m_tail.m_prev
        .fmap([&](auto&& prev) {
            return std::ref(prev.get().get());
        });
}

template<typename T>
Maybe<typename IntrusiveList<T>::CRef>
IntrusiveList<T>::back() const noexcept {
    if (empty()) {
        return none();
    }
    return m_tail.m_prev
        .fmap([&](auto&& prev) {
            return std::cref(prev.get().get());
        });
}

template<typename T>
bool IntrusiveList<T>::empty() const noexcept {
    return m_head.m_next.fmap([&](auto&& next) {
        return &next.get() == &m_tail;
    }).value_or_call([] {
        std::abort();
        return false;
    });
}

template<typename T>
void IntrusiveList<T>::do_move(IntrusiveList<T>& other) noexcept {
    m_head.m_next = other.m_head.m_next;
    m_head.m_next.fmap([&](auto&& next) {
        next.get().m_prev = LinkRef{m_head};
        return Unit{};
    });
    m_tail.m_prev = other.m_tail.m_prev;
    m_tail.m_prev.fmap([&](auto&& prev) {
        prev.get().m_next = LinkRef{m_tail};
        return Unit{};
    });
    other.m_head.m_next = none();
    other.m_tail.m_prev = none();
    other.m_tail.place_after(other.m_head);
}

}
