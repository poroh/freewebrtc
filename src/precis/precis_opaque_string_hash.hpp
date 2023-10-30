//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Hash for OpaqueString
//

#include <functional>
#include "precis/precis_opaque_string.hpp"

namespace freewebrtc::precis {

struct OpaqueStringHash {
    size_t operator()(const OpaqueString& s) const noexcept;
};

//
// Implementation
//
inline size_t OpaqueStringHash::operator()(const OpaqueString& s) const noexcept {
    std::hash<std::string> hasher;
    return hasher(s.value);
}

}
