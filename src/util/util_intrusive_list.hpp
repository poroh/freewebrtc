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
#include <optional>

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
    std::optional<Ref> front() noexcept;
    std::optional<CRef> front() const noexcept;
    std::optional<Ref> back() noexcept;
    std::optional<CRef> back() const noexcept;

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
    std::optional<Ref> m_owner;
    std::optional<LinkRef> m_next;
    std::optional<LinkRef> m_prev;
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
    if (m_prev.has_value()) {
        m_prev.value().get().m_next = *this;
    }
    if (m_next.has_value()) {
        m_next.value().get().m_prev = *this;
    }
    other.m_next.reset();
    other.m_prev.reset();
}

template<typename T>
IntrusiveList<T>::Link::~Link() {
    remove();
}

template<typename T>
T& IntrusiveList<T>::Link::get() noexcept {
    if (!m_owner.has_value()) {
        std::abort();
    }
    return m_owner.value();
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
    if (m_prev.has_value()) {
        m_prev.value().get().m_next = m_next;
    }
    if (m_next.has_value()) {
        m_next.value().get().m_prev = m_prev;
    }
    m_prev.reset();
    m_next.reset();
}

template<typename T>
void IntrusiveList<T>::Link::place_after(Link& t) noexcept {
    remove();
    m_next = t.m_next;
    m_prev = t;
    if (m_next.has_value()) {
        m_next.value().get().m_prev = *this;
    }
    m_prev.value().get().m_next = *this;
}

template<typename T>
void IntrusiveList<T>::Link::place_before(Link& t) noexcept {
    remove();
    m_next = t;
    m_prev = t.m_prev;
    m_next.value().get().m_prev = *this;
    if (m_prev.has_value()) {
        m_prev.value().get().m_next = *this;
    }
}

template<typename T>
bool IntrusiveList<T>::Link::in_list() const noexcept {
    return m_next.has_value() && m_prev.has_value();
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
    m_head.m_next.value().get().remove();
}

template<typename T>
void IntrusiveList<T>::pop_back() noexcept {
    if (empty()) {
        std::abort();
    }
    m_tail.m_prev.value().get().remove();
}

template<typename T>
std::optional<typename IntrusiveList<T>::Ref>
IntrusiveList<T>::front() noexcept {
    if (empty()) {
        return std::nullopt;
    }
    return m_head.m_next.value().get().get();
}

template<typename T>
std::optional<typename IntrusiveList<T>::CRef>
IntrusiveList<T>::front() const noexcept {
    if (empty()) {
        return std::nullopt;
    }
    return m_head.m_next.value().get().get();
}

template<typename T>
void IntrusiveList<T>::clear() noexcept {
    while (!empty()) {
        pop_front();
    }
}

template<typename T>
std::optional<typename IntrusiveList<T>::Ref>
IntrusiveList<T>::back() noexcept {
    if (empty()) {
        return std::nullopt;
    }
    return m_tail.m_prev.value().get().get();
}

template<typename T>
std::optional<typename IntrusiveList<T>::CRef>
IntrusiveList<T>::back() const noexcept {
    if (empty()) {
        return std::nullopt;
    }
    return m_tail.m_prev.value().get().get();
}

template<typename T>
bool IntrusiveList<T>::empty() const noexcept {
    if (!m_head.m_next.has_value() || !m_tail.m_prev.has_value()) {
        std::abort();
    }
    return &m_head.m_next.value().get() == &m_tail;
}

template<typename T>
void IntrusiveList<T>::do_move(IntrusiveList<T>& other) noexcept {
    m_head.m_next = other.m_head.m_next;
    m_head.m_next.value().get().m_prev = m_head;
    m_tail.m_prev = other.m_tail.m_prev;
    m_tail.m_prev.value().get().m_next = m_tail;
    other.m_head.m_next.reset();
    other.m_tail.m_prev.reset();
    other.m_tail.place_after(other.m_head);
}

}
