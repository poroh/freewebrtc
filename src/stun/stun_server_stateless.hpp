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
#include "precis/precis_opaque_string_hash.hpp"

namespace freewebrtc::stun::server {

class Stateless {
public:
    explicit Stateless(crypto::SHA1Hash::Func);
    using MaybeResponse = std::optional<Message>;
    MaybeResponse process(const net::Endpoint&, const util::ConstBinaryView&);
    void add_user(const precis::OpaqueString& name, const stun::Password&);

private:
    MaybeResponse process_request(const net::Endpoint&, const Message&, const util::ConstBinaryView&);

    const crypto::SHA1Hash::Func m_sha1;
    stun::ParseStat m_stat;
    using UserMap = std::unordered_map<precis::OpaqueString, stun::Password, precis::OpaqueStringHash>;
    UserMap m_users;
};

}
