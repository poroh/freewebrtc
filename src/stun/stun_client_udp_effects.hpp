//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN client effects
//

#pragma once

#include <vector>
#include <system_error>
#include <variant>

#include "util/util_maybe.hpp"
#include "util/util_binary_view.hpp"
#include "net/net_endpoint.hpp"
#include "clock/clock_timepoint.hpp"
#include "stun/stun_attribute_type.hpp"
#include "stun/stun_client_udp_handle.hpp"
#include "stun/stun_attribute.hpp"

namespace freewebrtc::stun::client_udp {

using Duration = clock::NativeDuration;

// SendData returns const references to message data
// and maybe integrity data. Lifetime of these reference
// guaranteed until TransactionFail / TransactionOk with the
// same handle received
struct SendData {
    Handle handle;
    util::ConstBinaryView message_view;
};

struct TransactionOk {
    Handle handle;
    // External IP address / Port
    net::UdpEndpoint result;
    // Parsed response message
    Message response;
    // Round-trip time maybe not calculated if
    // we used retransmissions and no cerainty about attributing
    // response to act of sending request.
    Maybe<Duration> round_trip;
};

struct TransactionFailed {
    // Response contains attributes that requires
    // comprehension but we don't know it.
    struct UnknownComprehensionRequiredAttribute {
        std::vector<AttributeType> attrs;
    };
    // UNKNOWN-ATTRIBUTE attribute value of 420 error code
    struct UnknownAttributeReported {
        std::vector<AttributeType> attrs;
    };
    // ALTERNATE-SERVER meachanism in use
    struct AlternateServer {
        net::UdpEndpoint server;
    };
    // Error code
    struct ErrorCode {
        ErrorCodeAttribute attr;
    };
    // Error occurred during transaction processing
    struct Error {
        ::freewebrtc::Error code;
    };

    struct Timeout{};

    using Reason = std::variant<
        UnknownComprehensionRequiredAttribute,
        UnknownAttributeReported,
        AlternateServer,
        ErrorCode,
        Error,
        Timeout
        >;
    Handle handle;
    Reason reason;
};

struct Sleep { Duration sleep; };

// No more actions needed until next start() is called.
struct Idle {};

using Effect = std::variant<SendData, TransactionOk, TransactionFailed, Sleep, Idle>;

}
