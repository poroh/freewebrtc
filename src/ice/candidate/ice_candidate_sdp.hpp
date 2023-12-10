//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate in SDP
// RFC8839
//

#pragma once

#include "ice/candidate/ice_candidate.hpp"
#include "util/util_tagged_type.hpp"

namespace freewebrtc::ice::candidate {

struct UnsupportedTag{};
using Unsupported = util::TaggedType<std::string, UnsupportedTag>;
struct Supported {
    Candidate candidate;
    struct Extension {
        std::string att_name;
        std::string att_value;
    };
    using ExtensionVec = std::vector<Extension>;
    ExtensionVec extensions;
};
using SDPAttrParseResult = std::variant<Supported, Unsupported>;

ReturnValue<SDPAttrParseResult> parse_sdp_attr(std::string_view);

}
