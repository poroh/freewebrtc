//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE checklist
//

#pragma once

#include "ice/pair/ice_pair.hpp"

#include <vector>

namespace freewebrtc::ice::checklist {

class Checklist {
public:
    using Candidate = ice::candidate::Candidate;
    void add_local(Candidate&&);
    void add_remote(Candidate&&);

private:
    using Pair = ice::pair::Pair;
    using ComponentId = ice::candidate::ComponentId;
    using CandidateVec = std::vector<ice::candidate::Candidate>;
    using PairVec = std::vector<ice::pair::Pair>;
    std::unordered_map<ComponentId, CandidateVec> m_local;
    std::unordered_map<ComponentId, CandidateVec> m_remote;
    std::unordered_map<ComponentId, PairVec> m_pairs;
};

}
