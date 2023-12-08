//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE ABNF helpers
// RFC8839
//

#pragma once

#include "abnf/abnf.hpp"

namespace freewebrtc::ice::abnf {

// ice-char  = ALPHA / DIGIT / "+" / "/"
bool is_ice_char(char c);

//
// implementation
//
inline bool is_ice_char(char c) {
    return freewebrtc::abnf::is_ALPHA(c)
        || freewebrtc::abnf::is_DIGIT(c)
        || c == '+' || c == '/';
}

}
