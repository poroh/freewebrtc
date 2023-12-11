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
#include "abnf/abnf.hpp"

namespace freewebrtc::ice::candidate {

namespace {

// RFC8839:
// This specification defines the values "host", "srflx",
// "prflx", and "relay" for host, server-reflexive,
// peer-reflexive, and relayed candidates, respectively.
const std::string host_str = "host";
const std::string server_reflexive_str = "srflx";
const std::string peer_reflexive_str = "prflx";
const std::string relayed_str = "relay";

// This just prevents from warning by gcc:
const std::string unknown_str = "unknown";

}

ReturnValue<Type> Type::from_string(const std::string_view& v) noexcept {
    if (abnf::eq_string(v, host_str)) {
        return host();
    } else if (abnf::eq_string(v, server_reflexive_str)) {
        return server_reflexive();
    } else if (abnf::eq_string(v, peer_reflexive_str)) {
        return peer_reflexive();
    } else if (abnf::eq_string(v, relayed_str)) {
        return relayed();
    }
    return ::freewebrtc::Error{make_error_code(Error::unknown_candidate_type)}
        .add_context("candidate type", v);
}

const std::string& Type::to_string() const noexcept {
    switch (m_value) {
    case HOST: return host_str;
    case SERVER_REFLEXIVE: return server_reflexive_str;
    case PEER_REFLEXIVE: return peer_reflexive_str;
    case RELAYED: return relayed_str;
    }
    return unknown_str;
}

}
