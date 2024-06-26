//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Tagged type is just wrapper around value
//

#pragma once

#include <utility>

namespace freewebrtc::util {

struct UnitTag{};

template<typename T, typename TagV = UnitTag>
struct TaggedType {
    using Value = T;
    using Tag = TagV;

    T value;

    TaggedType() = default;
    TaggedType(const TaggedType&) = default;
    TaggedType(TaggedType&&) = default;
    explicit TaggedType(const T&);
    explicit TaggedType(T&&);

    static TaggedType move_from(T&&);
    static TaggedType copy_from(const T&);

    TaggedType& operator=(const TaggedType&) = default;
    TaggedType& operator=(TaggedType&&) = default;

    bool operator==(const TaggedType&) const = default;
};

//
// inlines
//
template<typename T, typename Tag>
inline TaggedType<T, Tag>::TaggedType(const T& v)
    : value(v)
{}

template<typename T, typename Tag>
inline TaggedType<T, Tag>::TaggedType(T&& v)
    : value(std::move(v))
{}

template<typename T, typename Tag>
inline TaggedType<T, Tag> TaggedType<T, Tag>::move_from(T&& t) {
    return TaggedType{std::move(t)};
}
template<typename T, typename Tag>
inline TaggedType<T, Tag> TaggedType<T, Tag>::copy_from(const T& t) {
    return TaggedType{t};
}

}

