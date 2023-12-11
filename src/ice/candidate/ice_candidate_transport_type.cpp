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
#include "abnf/abnf.hpp"

namespace freewebrtc::ice::candidate {

namespace {

const std::string udp_str = "udp";

// This just prevents from warning by gcc:
const std::string unknown_str = "unknown";

}

ReturnValue<TransportType> TransportType::from_string(const std::string_view& v) noexcept {
    // RFC8839:
    // This specification only defines UDP
    if (abnf::eq_string(v, udp_str)) {
        return udp();
    }
    return ::freewebrtc::Error{make_error_code(Error::unknown_transport_type)}
        .add_context("transport type", v);
}

const std::string& TransportType::to_string() const {
    switch (m_value) {
    case UDP: return udp_str;
    }

    return unknown_str;
}


}
