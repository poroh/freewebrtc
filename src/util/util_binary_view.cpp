//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Binary packet view (does not contain own data, just works over existing buffer)
//

#include <numeric>
#include "util/util_binary_view.hpp"

namespace freewebrtc::util {

ConstBinaryView::ByteVec ConstBinaryView::concat(const std::vector<ConstBinaryView>& views) {
    size_t sz = std::accumulate(views.begin(), views.end(),
                                size_t{0},
                                [](size_t acc, const auto& v) { return acc + v.size(); });
    ConstBinaryView::ByteVec result(sz);
    auto pos = result.begin();
    for (const auto& v: views) {
        pos = std::copy(v.begin(), v.end(), pos);
    }
    return result;
}

}
