//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Type-specific boolean (not comparable/assignable to other bools).
//

#pragma once

namespace freewebrtc::util {

template<typename Tag>
class TypedBool {
public:
    explicit TypedBool(bool);
    operator bool() const noexcept;
private:
    bool value_;
};

template<typename Tag>
inline TypedBool<Tag>::TypedBool(bool v)
    : value_(v)
{}

template<typename Tag>
inline TypedBool<Tag>::operator bool() const noexcept {
    return value_;
}

}
