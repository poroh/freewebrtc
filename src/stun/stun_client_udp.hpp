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
#include <memory>

#include "clock/clock_timepoint.hpp"
#include "precis/precis_opaque_string.hpp"
#include "util/util_result.hpp"
#include "util/util_maybe.hpp"
#include "util/util_hash_dynamic.hpp"
#include "stun/stun_message.hpp"
#include "stun/stun_attribute_set.hpp"
#include "stun/stun_parse_stat.hpp"
#include "stun/stun_client_udp_settings.hpp"
#include "stun/stun_client_udp_handle.hpp"
#include "stun/stun_client_udp_effects.hpp"
#include "net/net_path.hpp"

namespace freewebrtc::stun::details { class ClientUDPRtoCalculator; }

namespace freewebrtc::stun {

using namespace std::literals::chrono_literals;

class ClientUDP {
public:
    using Timepoint = clock::Timepoint;
    using MaybeTimepoint = Maybe<Timepoint>;
    using Settings   = client_udp::Settings;
    using Handle     = client_udp::Handle;
    using HandleHash = client_udp::HandleHash;

    // Effects:
    using Effect            = client_udp::Effect;
    using SendData          = client_udp::SendData;
    using TransactionOk     = client_udp::TransactionOk;
    using TransactionFailed = client_udp::TransactionFailed;
    using Sleep             = client_udp::Sleep;
    using Idle              = client_udp::Idle;

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


    explicit ClientUDP(const Settings&);
    ~ClientUDP();

    // Authenticaiton information. If no specified
    // the message is created without username / integrity
    // attributes.
    struct Auth {
        precis::OpaqueString username;
        IntegrityData integrity;
    };
    using MaybeAuth = Maybe<Auth>;
    struct Request {
        net::Path path;
        AttributeSet::AttrVec attrs;
        AttributeSet::UnknownAttrVec unknown_attrs;
        MaybeAuth maybe_auth = None{};
    };
    // Create transaction with unique identifier. Random device
    // may be override with cryptographic random if needed.
    template<typename RandomDevice = std::random_device>
    Result<Handle> create(RandomDevice&, Timepoint now, Request&&);

    // Process response. If stun message was already parsed before
    // then caller may provide parsed message. It optimizes performance by
    // prventing another parse. However if auth is defined integrity will be checked
    // in any case by binary view.
    MaybeError response(Timepoint, util::ConstBinaryView, Maybe<stun::Message>&& = None{});

    // Do next step of the client processing
    Effect next(Timepoint);

private:
    using Duration = clock::NativeDuration;
    using TransactionIdHash = util::hash::dynamic::Hash<TransactionId>;
    class RetransmitAlgo;
    using RetransmitAlgoPtr = std::unique_ptr<RetransmitAlgo>;
    struct Transaction {
        ~Transaction();
        Transaction(Timepoint now, TransactionId&&, util::ByteVec&& msg_data, RetransmitAlgoPtr&& rtx_algo, net::Path&&, MaybeAuth&&);
        Transaction(const Transaction&) = delete;
        Transaction(Transaction&&) = default;

        TransactionId tid;
        util::ByteVec msg_data;
        RetransmitAlgoPtr rtx_algo;
        net::Path path;
        Timepoint create_time;
        MaybeAuth maybe_auth;
        unsigned rtx_count = 0;
    };
    using TimelineItem = std::pair<Timepoint, Handle>;
    struct TimelineGreater {
        bool operator()(const TimelineItem&, const TimelineItem&) const noexcept;
    };
    using RtoCalculator = details::ClientUDPRtoCalculator;
    using RtoCalculatorPtr = std::unique_ptr<RtoCalculator>;

    Result<Handle> do_create(Timepoint, TransactionId&&, Request&&);
    MaybeError handle_success_response(Timepoint now, Handle hnd, Message& msg);
    MaybeError handle_error_response(Timepoint now, Handle hnd, const Message& msg);
    Handle allocate_handle() noexcept;
    RetransmitAlgoPtr allocate_rtx_algo(const net::Path& path, Timepoint now);
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
    std::queue<Effect> m_effects;
    // Initial RTO calculator
    RtoCalculatorPtr m_rto_calc;
};

//
// implementation
//
template<typename RandomDevice>
inline Result<ClientUDP::Handle>
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

