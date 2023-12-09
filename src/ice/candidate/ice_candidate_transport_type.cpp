//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Type
// RFC8845
//

#include "ice/candidate/ice_candidate_transport_type.hpp"
#include "ice/candidate/ice_candidate_error.hpp"

namespace freewebrtc::ice::candidate {

ReturnValue<TransportType> TransportType::from_string(const std::string_view& v) noexcept {
    // RFC8839:
    // This specification only defines UDP
    if (v == "UDP") {
        return udp();
    }
    return ::freewebrtc::Error{make_error_code(Error::unknown_transport_type)}
        .add_context("transport type", v);
}

}
