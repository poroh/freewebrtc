//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN client over UDP
//

#pragma once

#include <chrono>
#include <random>
#include <functional>
#include <queue>

#include "clock/clock_timepoint.hpp"
#include "precis/precis_opaque_string.hpp"
#include "util/util_return_value.hpp"
#include "util/util_hash_dynamic.hpp"
#include "stun/stun_message.hpp"
#include "stun/stun_attribute_set.hpp"
#include "stun/stun_parse_stat.hpp"
#include "net/net_endpoint.hpp"

namespace freewebrtc::stun {

using namespace std::literals::chrono_literals;

class ClientUDP {
public:
    using Timepoint = clock::Timepoint;
    using Duration = clock::NativeDuration;
    using MaybeTimepoint = std::optional<Timepoint>;
    using TransactionIdHash = util::hash::dynamic::Hash<TransactionId>;

    struct Settings {
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
        // Default is RFC5389-defined mechanism
        struct RetransmitDefault {
            // Initial retransmit timeout
            Duration initial_rto = 500ms;
            // Maximum of retransmission interval.
            // In libwebrtc this parameter is set to 8s
            std::optional<clock::NativeDuration> max_rto = std::nullopt;
            // Request count (Rc)
            unsigned request_count = 7;
            // Retransmission multiplier for last request (Rm)
            unsigned retransmission_multiplier = 16;
            // 5xx handling. If not defined then transaction
            // is failed instantly
            std::optional<Duration> server_error_timeout;
            // Maximum number of retransmits in case of 5xx
            // responses
            unsigned server_error_max_retransmits = 5;
        };
        using Retransmit = std::variant<RetransmitDefault>;
        Retransmit retransmit = RetransmitDefault{};

        // Allow alternate server error without authentication
        // This case is described in RFC5389 in section
        // 11.  ALTERNATE-SERVER Mechanism
        bool allow_unauthenticated_alternate = false;

        // Hash of transaction ID. By default murmur hash without
        // seed randomization.
        std::optional<TransactionIdHash> maybe_tid_hash;
    };

    struct Statistics {
        ParseStat parse;
        stat::Counter started;
        stat::Counter success;
        stat::Counter retransmits;
        stat::Counter hash_calc_errors;
        stat::Counter integrity_missing;
        stat::Counter integrity_check_errors;
        stat::Counter transaction_not_found;
        stat::Counter unknown_attribute;
        stat::Counter no_error_code;
        stat::Counter try_alternate_responses;
        stat::Counter no_alternate_server_attr;
        stat::Counter response_3xx;
        stat::Counter response_4xx;
        stat::Counter response_5xx;
        stat::Counter unexpected_response_code;
        stat::Counter no_mapped_address;
    };

    struct Handle {
        unsigned value;
        bool operator<=>(const Handle&) const noexcept = default;
    };

    struct HandleHash {
        std::size_t operator()(Handle) const noexcept;
    };

    explicit ClientUDP(const Settings&);

    struct Request {
        net::UdpEndpoint target;
        AttributeSet::AttrVec attrs;
        AttributeSet::UnknownAttrVec unknown_attrs;
    };
    // Create transaction with unique identifier. Random device
    // may be override with cryptographic random if needed.
    template<typename RandomDevice = std::random_device>
    ReturnValue<Handle> create(RandomDevice&, Timepoint now, Request&&);

    // Process response. If stun message was already parsed before
    // then caller may provide parsed message. It optimizes performance by
    // prventing another parse. However if auth is defined integrity will be checked
    // in any case by binary view.
    MaybeError response(Timepoint, util::ConstBinaryView, std::optional<stun::Message>&& = {});

    // SendData returns const references to message data
    // and maybe integrity data. Lifetime of these reference
    // guaranteed until next call of next() function.
    struct SendData {
        Handle handle;
        util::ConstBinaryView message_view;
    };

    struct TransactionOk {
        Handle handle;
        // External IP address / Port
        net::UdpEndpoint result;
        // Parsed response message
        Message msg;
        // Round-trip time maybe not calculated if
        // we used retransmissions and no cerainty about attributing
        // response to act of sending request.
        std::optional<Duration> round_trip;
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
            std::error_code code;
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

    using Next = std::variant<SendData, TransactionOk, TransactionFailed, Sleep, Idle>;

    // Do next step of the client processing
    Next next(Timepoint);

private:
    class RetransmitAlgo;
    using RetransmitAlgoPtr = std::unique_ptr<RetransmitAlgo>;
    struct Transaction {
        ~Transaction();
        Transaction(Timepoint now, TransactionId&&, util::ByteVec&& msg_data, RetransmitAlgoPtr&& rtx_algo);
        Transaction(const Transaction&) = delete;
        Transaction(Transaction&&) = default;

        TransactionId tid;
        util::ByteVec msg_data;
        RetransmitAlgoPtr rtx_algo;
        Timepoint create_time;
        unsigned rtx_count = 0;
    };
    using TimelineItem = std::pair<Timepoint, Handle>;
    struct TimelineGreater {
        bool operator()(const TimelineItem&, const TimelineItem&) const noexcept;
    };

    ReturnValue<Handle> do_create(Timepoint, TransactionId&&, Request&&);
    MaybeError handle_success_response(Timepoint now, Handle hnd, Message& msg);
    MaybeError handle_error_response(Timepoint now, Handle hnd, const Message& msg);
    Handle allocate_handle() noexcept;
    RetransmitAlgoPtr allocate_rtx_algo(Timepoint now);
    void cleanup(const Handle&);

    const Settings m_settings;
    Statistics m_stat;
    decltype(Handle::value) m_next_handle_value = 0;
    // Way to find Handle by Transaction Id
    std::unordered_map<TransactionId, Handle, TransactionIdHash> m_tid_to_handle;
    // Primary storage of the transaction states.
    std::unordered_map<Handle, Transaction, HandleHash> m_tmap;
    // Next timeline across all existing transacitons.
    std::priority_queue<TimelineItem, std::vector<TimelineItem>,  TimelineGreater> m_tid_timeline;
    // Pending effects that returned to user from next() function.
    std::queue<Next> m_effects;
};

//
// implementation
//
inline std::size_t ClientUDP::HandleHash::operator()(Handle hnd) const noexcept {
    std::hash<decltype(hnd.value)> h;
    return h(hnd.value);
}

template<typename RandomDevice>
inline ReturnValue<ClientUDP::Handle>
ClientUDP::create(RandomDevice& rand, Timepoint now, Request&& req) {
    while (true) {
        TransactionId id = TransactionId::generate(rand);
        if (m_tid_to_handle.contains(id)) {
            continue;
        }
        return do_create(now, std::move(id), std::move(req));
    }
}

}

