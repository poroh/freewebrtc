//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ABNF helpers
// RFC5234
//

#pragma once

#include <string_view>
#include <cctype>

namespace freewebrtc::abnf {

// ALPHA = %x41-5A / %x61-7A   ; A-Z / a-z
bool is_ALPHA(char c);
// DIGIT  =  %x30-39
bool is_DIGIT(char c);

// 2.3.  Terminal Values
// ABNF strings are case insensitive and the character set for these
// strings is US-ASCII.
bool eq_string(std::string_view, std::string_view);

//
// implementation
//
inline bool is_ALPHA(char c) {
    return (0x41 <= c && c <= 0x5a)
        || (0x61 <= c && c <= 0x7a);
}

inline bool is_DIGIT(char c) {
    return 0x30 <= c && c <= 0x39;
}

inline bool eq_string(std::string_view v1, std::string_view v2) {
    if (v1.size() != v2.size()) {
        return false;
    }
    for (std::string_view::size_type i = 0; i < v1.size(); ++i) {
        if (std::tolower(v1[i]) != std::tolower(v2[i])) {
            return false;
        }
    }
    return true;
}


}
