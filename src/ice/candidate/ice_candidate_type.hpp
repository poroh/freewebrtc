//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate Type
// RFC8845
//

#pragma once

namespace freewebrtc::ice::candidate {

class Type {
public:
    enum Value {
        HOST,
        SERVER_REFLEXIVE,
        PEER_REFLEXIVE,
        RELAYED
    };
    static Type host();
    static Type server_reflexive();
    static Type peer_reflexive();
    static Type relayed();

private:
    Value m_value;
};

}
