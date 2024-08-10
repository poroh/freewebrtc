//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// ICE checklist
//

#include <algorithm>

#include "ice/checklist/ice_checklist.hpp"

namespace freewebrtc::ice::checklist {

void Checklist::add_local(Candidate&& c) {
    auto [it, _] = m_local.emplace(c.component, CandidateVec());
    it->second.emplace_back(std::move(c));
}

void Checklist::add_remote(Candidate&& c) {
    auto [it, _] = m_remote.emplace(c.component, CandidateVec());
    it->second.emplace_back(std::move(c));
}

}
