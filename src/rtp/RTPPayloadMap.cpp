//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Map
//

#include "rtp/RTPPayloadMap.hpp"

namespace freewebrtc::rtp {

PayloadMap::PayloadMap(PairInitializer l)
    : m_items(l.begin(), l.end())
{}


}


