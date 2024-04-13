//
// Copyright (c) 2024 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Reduce elements of containers
//

#include <util/util_result.hpp>

namespace freewebrtc::util {

template<typename Iter, typename Accumulator, typename Function>
Result<Accumulator> reduce(Iter start, Iter end, Accumulator acc, Function f) {
    for (; start != end; ++start) {
        if (auto next = f(std::move(acc), *start); next.is_err()) {
            return next.unwrap_err();
        }
    }
    return acc;
}

template<typename Iter, typename Function>
MaybeError reduce(Iter start, Iter end, Function f) {
    auto wf = [&](auto&&, auto&& v) {
        return f(v);
    };
    return reduce(start, end, Unit{}, wf);
}

}

