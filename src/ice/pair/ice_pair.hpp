//
// Copyright (c) 2024 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Pair (RFC8445)
//

#pragma once

#include "ice/candidate/ice_candidate.hpp"
#include "ice/pair/ice_pair_state.hpp"
#include "util/util_tagged_type.hpp"

namespace freewebrtc::ice::pair {

struct PriorityTag{};
using Priority = util::TaggedType<uint64_t, PriorityTag>;

struct Pair {
    candidate::Candidate local;
    candidate::Candidate remote;
    State state;

    // Priority of the pair if this agengent is controlling agent.
    Priority controlling_agent() const noexcept;
    // Priority of the pair if this agengent is controlled agent.
    Priority controlled_agent() const noexcept;
};

}
