//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Flattening of different containers
//

#pragma once

#include <algorithm>
#include <type_traits>

namespace freewebrtc::util {

template<typename T>
std::vector<T> flat_vec(const std::vector<std::vector<T>>& v) {
    std::vector<T> result;
    for (const auto& container : v) {
        std::copy(container.begin(), container.end(), std::back_inserter(result));
    }
    return result;
}

}
