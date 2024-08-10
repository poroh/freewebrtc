//
// Copyright (c) 2024 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Pair (RFC8445)
//

#include "ice/pair/ice_pair.hpp"

namespace freewebrtc::ice::pair {

// Let G be the priority for the candidate provided by the controlling agent.
// Let D be the priority for the candidate provided by the controlled agent.
Priority calc_priority(candidate::Priority g, candidate::Priority d) {
    uint64_t G = g.value();
    uint64_t D = d.value();
    // pair priority = 2^32*MIN(G,D) + 2*MAX(G,D) + (G>D?1:0)
    return Priority{(G << 32) * std::min(G, D) + 2 * std::max(G, D) + (G > D ? 1 : 0)};
}

Priority Pair::controlling_agent() const noexcept {
    return calc_priority(local.priority, remote.priority);
}

Priority Pair::controlled_agent() const noexcept {
    return calc_priority(remote.priority, local.priority);
}

}
