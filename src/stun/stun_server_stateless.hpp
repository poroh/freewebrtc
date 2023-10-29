//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Stateless server implementation
//

#pragma once

#include "stun/stun_message.hpp"
#include "net/net_endpoint.hpp"

namespace freewebrtc::stun::server {

class Stateless {
public:
    using Response = std::optional<Message>;
    Response create_response(const net::Endpoint&, const Message&);
};

}
