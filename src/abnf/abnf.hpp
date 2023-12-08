//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ABNF helpers
// RFC5234
//

#pragma once

namespace freewebrtc::abnf {

// ALPHA = %x41-5A / %x61-7A   ; A-Z / a-z
bool is_ALPHA(char c);
// DIGIT  =  %x30-39
bool is_DIGIT(char c);

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

}
