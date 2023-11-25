//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN client over UDP
//

#pragma once

#include <optional>
#include <variant>
#include <chrono>
#include "util/util_typed_bool.hpp"
#include "clock/clock_timepoint.hpp"
#include "precis/precis_opaque_string.hpp"
#include "stun/stun_transaction_id_hash.hpp"
#include "stun/stun_integrity.hpp"

namespace freewebrtc::stun::client_udp {

using namespace std::chrono_literals;

struct Settings {
    using Duration = clock::NativeDuration;

    // Authenticaiton information. If no specified
    // the message is created without username / integrity
    // attributes.
    struct Auth {
        precis::OpaqueString username;
        IntegrityData integrity;
    };
    std::optional<Auth> maybe_auth;

    // Use FINGERPRINT mechanism of RFC5389
    struct UseFingerprintTag{};
    using UseFingerprint = util::TypedBool<UseFingerprintTag>;
    UseFingerprint use_fingerprint = UseFingerprint{true};

    // Possible retransmit mechanisms settings.
    // Default is RFC8489-defined mechanism
    struct RetransmitDefault {
        // Maximum of retransmission interval.
        // In libwebrtc this parameter is set to 8s
        std::optional<Duration> max_rto = std::nullopt;
        // Request count (Rc)
        unsigned request_count = 7;
        // Retransmission multiplier for last request (Rm)
        unsigned retransmission_multiplier = 16;
        // 5xx handling. If not defined then transaction
        // is failed instantly
        std::optional<Duration> server_error_timeout;
        // Maximum number of retransmits in case of 5xx
        // RFC 8490: 6.3.4.  Processing an Error Response
        // If the error code is 500 through 599, the client MAY resend the
        // request; clients that do so MUST limit the number of times they do
        // this.  Unless a specific error code specifies a different value,
        // the number of retransmissions SHOULD be limited to 4.
        unsigned server_error_max_retransmits = 4;
    };
    using Retransmit = std::variant<RetransmitDefault>;
    Retransmit retransmit = RetransmitDefault{};

    struct RtoCalculatorSettings {
        // Initial retransmit timeout
        Duration initial_rto = 500ms;
        // How long do we keep history for each individual
        // network path
        Duration history_duration = 1h;
    };

    RtoCalculatorSettings rto_settings = {};

    // Allow alternate server error without authentication
    // This case is described in RFC5389 in section
    // 11.  ALTERNATE-SERVER Mechanism
    bool allow_unauthenticated_alternate = false;

    // Hash of transaction ID. By default murmur hash without
    // seed randomization.
    std::optional<TransactionIdHash> maybe_tid_hash;
};

}
