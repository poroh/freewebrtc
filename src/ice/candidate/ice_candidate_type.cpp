//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Type
// RFC8845
//

#include "ice/candidate/ice_candidate_type.hpp"
#include "ice/candidate/ice_candidate_error.hpp"

namespace freewebrtc::ice::candidate {

ReturnValue<Type> Type::from_string(const std::string_view& v) noexcept {
    if (v == "host") {
        return host();
    } else if (v == "srflx") {
        return server_reflexive();
    } else if (v == "prflx") {
        return peer_reflexive();
    } else if (v == "relay") {
        return relayed();
    }
    return ::freewebrtc::Error{make_error_code(Error::unknown_candidate_type)}
        .add_context("candidate type", v);
}

}
