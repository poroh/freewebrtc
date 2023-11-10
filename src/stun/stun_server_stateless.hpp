//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Stateless server implementation
//

#pragma once

#include "stun/stun_message.hpp"
#include "stun/stun_integrity.hpp"
#include "net/net_endpoint.hpp"
#include "precis/precis_opaque_string_hash.hpp"

namespace freewebrtc::stun::server {

class Stateless {
public:
    struct Settings {
        bool use_fingerprint = true;
    };
    Stateless(crypto::SHA1Hash::Func, const std::optional<Settings>& = std::nullopt);
    struct Respond {
        Message response;
        Message request;
        MaybeIntegrity maybe_integrity;
    };
    struct Error {
        std::error_code error;
    };
    struct Ignore {
        // Message is set if message is parsed as STUN message bu
        // it is not Request or has unknown method
        std::optional<Message> message;
    };
    using ProcessResult = std::variant<Respond, Ignore, Error>;

    ProcessResult process(const net::Endpoint&, const util::ConstBinaryView&);
    void add_user(const precis::OpaqueString& name, const stun::Password&);

private:
    ProcessResult process_request(const net::Endpoint&, Message&&, const util::ConstBinaryView&);

    const crypto::SHA1Hash::Func m_sha1;
    const Settings m_settings;
    stun::ParseStat m_stat;
    using UserMap = std::unordered_map<precis::OpaqueString, stun::Password, precis::OpaqueStringHash>;
    UserMap m_users;
};

}
