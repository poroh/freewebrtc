//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN client over UDP
//

#include <memory>
#include "stun/stun_client_udp.hpp"
#include "stun/stun_error.hpp"
#include "stun/stun_transaction_id_hash.hpp"
#include "stun/details/stun_client_udp_rto.hpp"

namespace freewebrtc::stun {

class ClientUDP::RetransmitAlgo {
public:
    explicit RetransmitAlgo(Duration initial_rto, const Settings::RetransmitDefault& settings, Timepoint now);
    MaybeTimepoint init(Timepoint now);
    MaybeTimepoint next(Timepoint now);
    enum class Process5xxResult {
        TransactionFailed,
        RetransmitScheduled
    };
    Process5xxResult process_5xx(Timepoint, int code);
    Duration last_timeout() const;
private:
    std::pair<MaybeTimepoint, Duration> calc_next(Timepoint now) const noexcept;
    const Duration m_initial_rto;
    const Settings::RetransmitDefault m_settings;
    MaybeTimepoint m_maybe_next;
    Duration m_last_timeout;
    unsigned m_rtx_count = 0;
    unsigned m_5xx_count = 0;
};

ClientUDP::ClientUDP(const Settings& settings)
    : m_settings(settings)
    , m_tid_to_handle(0,
        m_settings
          .maybe_tid_hash
          .value_or(MurmurTransactionIdHash<>::create()))
    , m_rto_calc(std::make_unique<RtoCalculator>(m_settings.rto_settings))
{}

ClientUDP::~ClientUDP()
{}

MaybeError ClientUDP::response(Timepoint now, util::ConstBinaryView view, Maybe<stun::Message>&& maybe_msg) {
    auto msg_rv = maybe_msg.require()
        .bind_err([&](auto&&) {
            return stun::Message::parse(view, m_stat.parse);
        });

    // Find transaction handle first not to spend time on
    // message validation if we don't know anything about
    // transaction
    auto trans_ref_rv = msg_rv
        .bind([&](const Message& msg) {
            return find_transaction(msg.header.transaction_id);
        });

    auto auth_err =
        combine([&](auto&& trans, const stun::Message& msg) {
            return check_response_auth(trans, msg, view);
        }, trans_ref_rv, msg_rv);

    return
        combine([&](auto&& trans, stun::Message&& msg, auto&&) {
            if (msg.header.cls == Class::success_response()) {
                return handle_success_response(now, trans.get(), std::move(msg));
            } else if (msg.header.cls == Class::error_response()) {
                return handle_error_response(now, trans.get(), std::move(msg));
            }
            return success();
        }, std::move(trans_ref_rv), std::move(msg_rv), std::move(auth_err));
}

ClientUDP::Effect ClientUDP::next(Timepoint now) {
    // Progress with pending transactions
    while (!m_tid_timeline.empty() && !m_tid_timeline.top().first.is_after(now)) {
        Handle hnd = m_tid_timeline.top().second;
        m_tid_timeline.pop();
        if (auto it = m_tmap.find(hnd); it != m_tmap.end()) {
            auto& t = it->second;
            auto se = t.rtx_algo->next(now)
                .fmap([&](auto&& next) -> Effect {
                    m_stat.retransmits.inc();
                    t.rtx_count++;
                    m_tid_timeline.emplace(std::make_pair(next, hnd));
                    return SendData{hnd, util::ConstBinaryView(t.msg_data)};
                })
                .value_or(TransactionFailed{hnd, TransactionFailed::Timeout{}});
            m_effects.emplace(std::move(se));
        } // When not found transaction were cleaned before.
    }
    if (!m_effects.empty()) {
        auto next = std::move(m_effects.front());
        m_effects.pop();
        if (auto* failed = std::get_if<TransactionFailed>(&next)) {
            cleanup(failed->handle);
        }
        if (auto* ok = std::get_if<TransactionOk>(&next)) {
            cleanup(ok->handle);
        }
        return next;
    }
    if (m_tid_timeline.empty()) {
        return Idle{};
    }
    return Sleep{m_tid_timeline.top().first - now};
}

Result<ClientUDP::TransactionRef> ClientUDP::find_transaction(const TransactionId& tid) noexcept {
    auto i = m_tid_to_handle.find(tid);
    if (i == m_tid_to_handle.end()) {
        m_stat.transaction_not_found.inc();
        return make_error_code(ClientError::transaction_not_found);
    }
    auto hnd = i->second;
    auto j = m_tmap.find(hnd);
    if (j == m_tmap.end()) {
        m_stat.transaction_not_found.inc();
        return make_error_code(ClientError::transaction_not_found);
    }
    return std::ref(j->second);
}

MaybeError ClientUDP::check_response_auth(const Transaction& trans, const Message& msg, util::ConstBinaryView view) noexcept {
    return trans.maybe_auth
        .fmap([&](auto&& auth) {
            // We don't check username because per:
            // RFC5389: 10.1.2.  Receiving a Request or Indication:
            // The response MUST NOT contain the USERNAME attribute.
            return msg.is_valid(view, auth.integrity)
                .bind([&](auto&& maybe_is_valid) -> MaybeError {
                    return maybe_is_valid
                        .fmap([&](auto is_valid) -> MaybeError {
                            // Response is signed
                            if (!is_valid) {
                                m_stat.integrity_check_errors.inc();
                                return make_error_code(ClientError::digest_is_not_valid);
                            }
                            return success();
                        })
                        .value_or_call([&]() -> MaybeError {
                            // Response is not signed
                            if (!m_settings.allow_unauthenticated_alternate || !msg.is_alternate_server()) {
                                m_stat.integrity_missing.inc();
                                return make_error_code(ClientError::no_integrity_attribute_in_response);
                            } // otherwise process as authenticated alternate server response
                            return success();
                        });
                });
        })
        .value_or(success());
}

Result<ClientUDP::Handle> ClientUDP::do_create(Timepoint now, TransactionId&& tid, Request&& rq) {
    stun::Message request {
        stun::Header {
            stun::Class::request(),
            stun::Method::binding(),
            std::move(tid)
        },
        stun::AttributeSet::create(std::move(rq.attrs), std::move(rq.unknown_attrs)),
        stun::IsRFC3489{false},
        none()
    };

    MaybeIntegrity maybe_integrity = rq.maybe_auth
        .fmap([&](auto&& auth) {
            request.attribute_set.emplace(Attribute::create(UsernameAttribute{auth.username}));
            return auth.integrity;
        });

    if (m_settings.use_fingerprint) {
        request.attribute_set.emplace(Attribute::create(FingerprintAttribute{0}));
    }

    return request
        .build(maybe_integrity)
        .fmap([&](util::ByteVec&& data) {
            Handle handle = allocate_handle();
            m_tid_to_handle.emplace(request.header.transaction_id, handle);

            auto rtx_algo = allocate_rtx_algo(rq.path, now);
            Transaction transaction(now,
                                    std::move(request.header.transaction_id),
                                    handle,
                                    std::move(data),
                                    std::move(rtx_algo),
                                    std::move(rq.path),
                                    std::move(rq.maybe_auth));
            auto [it, _] = m_tmap.emplace(handle, std::move(transaction));
            auto& t = it->second;
            m_effects.emplace(SendData{handle, util::ConstBinaryView(t.msg_data)});
            t.rtx_algo->init(now)
                .fmap([&](auto&& timepoint) {
                    m_tid_timeline.emplace(timepoint, handle);
                    return Unit{};
                });
            m_stat.started.inc();
            return handle;
        });

}

MaybeError ClientUDP::handle_success_response(Timepoint now, Transaction& t, Message&& msg) {
    // RFC5389:
    // 7.3.3.  Processing a Success Response
    // If the success response contains unknown
    // comprehension-required attributes, the response is
    // discarded and the transaction is considered to have failed.
    auto ucr = msg.attribute_set.unknown_comprehension_required();
    if (!ucr.empty()) {
        m_stat.unknown_attribute.inc();
        auto reason = TransactionFailed::UnknownComprehensionRequiredAttribute{std::move(ucr)};
        m_effects.emplace(TransactionFailed{t.hnd, std::move(reason)});
        return success();
    }

    Maybe<Duration> maybe_rtt = none();
    if (t.rtx_count == 0) {
        // Karn's algorithm to select retransmit timeout.
        // (http://ccr.sigcomm.org/archive/1995/jan95/ccr-9501-partridge87.pdf)
        // TLDR; Two ideas:
        //   1. Don't use RTT calculated based on retransmit
        //   2. Maintain back-off RTO when send next packet
        auto rtt = now - t.create_time;
        maybe_rtt = rtt;
        m_rto_calc->new_rtt(now, t.path, rtt);
    } else {
        m_rto_calc->backoff(now, t.path, t.rtx_algo->last_timeout());
    }

    auto effect = msg.attribute_set.xor_mapped()
        .fmap([&](auto&& xor_mapped_ref) -> Effect {
            m_stat.success.inc();
            auto& xor_mapped = xor_mapped_ref.get();
            net::UdpEndpoint ep{xor_mapped.addr.to_address(msg.header.transaction_id), xor_mapped.port};
            return TransactionOk{t.hnd, std::move(ep), std::move(msg), maybe_rtt};
        })
        .value_or_call([&] {
            return msg.attribute_set.mapped()
                .fmap([&](auto&& mapped_ref) -> Effect {
                    m_stat.success.inc();
                    auto& mapped = mapped_ref.get();
                    net::UdpEndpoint ep{mapped.addr, mapped.port};
                    return TransactionOk{t.hnd, std::move(ep), std::move(msg), maybe_rtt};
                })
                .value_or_call([&]() -> Effect {
                    m_stat.no_mapped_address.inc();
                    auto reason = TransactionFailed::Error{make_error_code(ClientError::no_address_in_response)};
                    return TransactionFailed{t.hnd, std::move(reason)};
                });
        });
    m_effects.emplace(effect);

    return success();
}

MaybeError ClientUDP::handle_error_response(Timepoint now, Transaction& t, Message&& msg) {
    // RFC5389:
    // 7.3.4.  Processing an Error Response
    // If the error response contains unknown comprehension-required
    // attributes, or if the error response does not contain an ERROR-CODE
    // attribute, then the transaction is simply considered to have failed.
    auto ucr = msg.attribute_set.unknown_comprehension_required();
    if (!ucr.empty()) {
        m_stat.unknown_attribute.inc();
        auto reason = TransactionFailed::UnknownComprehensionRequiredAttribute{std::move(ucr)};
        m_effects.emplace(TransactionFailed{t.hnd, std::move(reason)});
        return success();
    }

    const auto& maybe_msg_error_code = msg.attribute_set.error_code();
    if (maybe_msg_error_code.is_none()) {
        m_stat.no_error_code.inc();
        auto reason = TransactionFailed::Error{make_error_code(ClientError::no_error_code_in_response)};
        m_effects.emplace(TransactionFailed{t.hnd, std::move(reason)});
        return success();
    }
    const auto& msg_error_code = maybe_msg_error_code.unwrap().get();

    if (msg.is_alternate_server()) {
        m_stat.response_3xx.inc();
        const auto& maybe_alt_srv = msg.attribute_set.alternate_server();
        if (maybe_alt_srv.is_none()) {
            m_stat.no_alternate_server_attr.inc();
            auto reason = TransactionFailed::Error{make_error_code(ClientError::no_alternate_server_in_response)};
            m_effects.emplace(TransactionFailed{t.hnd, std::move(reason)});
            return success();
        }
        const auto& alt_srv = maybe_alt_srv.unwrap().get();
        m_stat.try_alternate_responses.inc();
        auto reason = TransactionFailed::AlternateServer{{alt_srv.addr, alt_srv.port}};
        m_effects.emplace(TransactionFailed{t.hnd, std::move(reason)});
        return success();
    }

    switch (msg_error_code.code / 100) {
    case 3: {
        // Normative:
        // If the error code is 300 through 399, the client SHOULD consider
        // the transaction as failed unless the ALTERNATE-SERVER extension is
        // being used. See Section 11.
        // Implementation:
        // Alternate server is checked above.
        m_stat.response_3xx.inc();
        auto reason = TransactionFailed::ErrorCode{msg_error_code};
        m_effects.emplace(TransactionFailed{t.hnd, std::move(reason)});
        return success();
    }
    case 4: {
        // If the error code is 400 through 499, the client declares the
        // transaction failed; in the case of 420 (Unknown Attribute), the
        // response should contain a UNKNOWN-ATTRIBUTES attribute that gives
        // additional information.
        m_stat.response_4xx.inc();
        if (msg_error_code.code == ErrorCodeAttribute::UnknownAttribute) {
            const auto& maybe_ua = msg.attribute_set.unknown_attributes();
            if (maybe_ua.is_some()) {
                auto reason = TransactionFailed::UnknownAttributeReported{maybe_ua.unwrap().get().types};
                m_effects.emplace(TransactionFailed{t.hnd, std::move(reason)});
                return success();
            }
        }
        auto reason = TransactionFailed::ErrorCode{msg_error_code};
        m_effects.emplace(TransactionFailed{t.hnd, std::move(reason)});
        return success();
    }
    case 5: {
        // If the error code is 500 through 599, the client MAY resend the
        // request; clients that do so MUST limit the number of times they do
        // this.
        m_stat.response_5xx.inc();
        switch (t.rtx_algo->process_5xx(now, msg_error_code.code)) {
        case RetransmitAlgo::Process5xxResult::RetransmitScheduled:
            return success();
        case RetransmitAlgo::Process5xxResult::TransactionFailed: {
            auto reason = TransactionFailed::ErrorCode{msg_error_code};
            m_effects.emplace(TransactionFailed{t.hnd, std::move(reason)});
            return success();
        }
        }
        return success();
    }
    }
    m_stat.unexpected_response_code.inc();
    return success();
}

ClientUDP::Handle ClientUDP::allocate_handle() noexcept {
    while (true) {
        if (auto hnd = Handle{m_next_handle_value++}; !m_tmap.contains(hnd)) {
            return hnd;
        }
    }
}

ClientUDP::RetransmitAlgoPtr ClientUDP::allocate_rtx_algo(const net::Path& path, Timepoint now) {
    auto rto = m_rto_calc->rto(path);
    return
        std::visit(
            util::overloaded {
                [&](const Settings::RetransmitDefault& settings) {
                    return std::make_unique<RetransmitAlgo>(rto, settings, now);
                }
            },
            m_settings.retransmit);
}

void ClientUDP::cleanup(const Handle& hnd) {
    if (auto it = m_tmap.find(hnd); it != m_tmap.end()) {
        const auto& tid = it->second.tid;
        m_tid_to_handle.erase(tid);
        m_tmap.erase(it);
        // We intentionally do not remove transaction from timeline if
        // it is there it will be just ignored on next.
    }
}

ClientUDP::Transaction::Transaction(Timepoint now, TransactionId&& t, Handle h, util::ByteVec&& data,
                                    RetransmitAlgoPtr&& algo, net::Path&& p, MaybeAuth&& a)
    : tid(std::move(t))
    , hnd(h)
    , msg_data(std::move(data))
    , rtx_algo(std::move(algo))
    , path(std::move(p))
    , create_time(now)
    , maybe_auth(std::move(a))
{}

ClientUDP::Transaction::~Transaction()
{}

bool ClientUDP::TimelineGreater::operator()(const TimelineItem& lhs, const TimelineItem& rhs) const noexcept {
    if (lhs.first.is_after(rhs.first)) {
        return true;
    }
    if (rhs.first.is_after(lhs.first)) {
        return false;
    }
    return lhs.second.value > rhs.second.value;
}

ClientUDP::RetransmitAlgo::RetransmitAlgo(Duration initial_rto, const Settings::RetransmitDefault& settings, Timepoint now)
    : m_initial_rto(initial_rto)
    , m_settings(settings)
    , m_maybe_next(now.advance(initial_rto))
    , m_last_timeout(initial_rto)
{}

ClientUDP::MaybeTimepoint ClientUDP::RetransmitAlgo::init(Timepoint now) {
    m_maybe_next = now.advance(m_last_timeout);
    return m_maybe_next;
}

ClientUDP::MaybeTimepoint ClientUDP::RetransmitAlgo::next(Timepoint now) {
    bool time_for_next = m_maybe_next
        .fmap([&](auto&& next) {
            return !now.is_before(next);
        })
        .value_or(false);
    if (time_for_next) {
        auto [maybe_next, timeout] = calc_next(now);
        m_maybe_next = maybe_next;
        if (maybe_next.is_some()) {
            ++m_rtx_count;
            m_last_timeout = timeout;
        }
    }
    return m_maybe_next;
}

ClientUDP::RetransmitAlgo::Process5xxResult
ClientUDP::RetransmitAlgo::process_5xx(Timepoint now, int) {
    return m_settings.server_error_timeout
        .fmap([&](auto timeout) {
            // If timeout is specified then do retransmits up to
            // server_error_max_retransmits.
            if (m_5xx_count >= m_settings.server_error_max_retransmits) {
                return Process5xxResult::TransactionFailed;
            }
            m_5xx_count++;
            m_maybe_next = now.advance(timeout);
            return Process5xxResult::RetransmitScheduled;
        })
        .value_or(Process5xxResult::TransactionFailed);
}

ClientUDP::Duration ClientUDP::RetransmitAlgo::last_timeout() const {
    return m_last_timeout;
}

std::pair<ClientUDP::MaybeTimepoint, client_udp::Duration> ClientUDP::RetransmitAlgo::calc_next(Timepoint now) const noexcept {
    if (m_rtx_count + 1 >= m_settings.request_count + m_5xx_count) {
        return std::make_pair(none(), Duration(0));
    }
    auto current = m_rtx_count + 2 == m_settings.request_count
        ? m_initial_rto * m_settings.retransmission_multiplier
        : m_last_timeout * 2;

    auto timeout = m_settings.max_rto
        .fmap([&](auto&& settings_max_rto) {
            return std::max(current, settings_max_rto);
        })
        .value_or(current);
    return std::make_pair(now.advance(timeout), timeout);
}

}

