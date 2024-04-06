//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate (RFC8445)
//

#pragma once

#include "net/net_endpoint.hpp"

#include "ice/candidate/ice_candidate_foundation.hpp"
#include "ice/candidate/ice_candidate_priority.hpp"
#include "ice/candidate/ice_candidate_component_id.hpp"
#include "ice/candidate/ice_candidate_type.hpp"
#include "ice/candidate/ice_candidate_transport_type.hpp"
#include "ice/candidate/ice_candidate_address.hpp"

namespace freewebrtc::ice::candidate {

struct Candidate {
    Address address;
    net::Port port;
    TransportType transport_type;
    Foundation foundation;
    ComponentId component;
    Priority priority;
    Type type;
    std::optional<Address> maybe_related_address;
    std::optional<net::Port> maybe_related_port;
};

}

