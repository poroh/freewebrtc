//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Error in freewebrtc library
//

#include "util/util_error.hpp"
#include "util/util_string_view.hpp"

namespace freewebrtc {

std::string Error::message() const {
    std::vector<std::string_view> out;
    for (auto rit = m_context.rbegin(); rit != m_context.rend(); ++rit) {
        out.emplace_back(*rit);
    }
    out.emplace_back(m_code.message());
    return util::string_view::join(out, ": ");
}

}
