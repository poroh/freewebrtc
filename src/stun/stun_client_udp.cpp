//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN client over UDP
//

#include "stun/stun_client_udp.hpp"
#include "stun/stun_error.hpp"

namespace freewebrtc::stun {

class ClientUDP::RetransmitAlgo {
public:
    explicit RetransmitAlgo(const Settings::RetransmitDefault& settings, Timepoint now);
    MaybeTimepoint next(Timepoint now);
    enum class Process5xxResult {
        TransactionFailed,
        RetransmitScheduled
    };
    Process5xxResult process_5xx(Timepoint, int code);
private:
    const Settings::RetransmitDefault m_settings;
    std::optional<MaybeTimepoint> m_next_rtx;
};

MaybeError ClientUDP::response(Timepoint now, util::ConstBinaryView view, std::optional<stun::Message>&& maybe_msg) {
    if (!maybe_msg.has_value()) {
        auto parsed_msg_rv = stun::Message::parse(view, m_stat.parse);
        if (parsed_msg_rv.is_error()) {
            return parsed_msg_rv.assert_error();
        }
        maybe_msg.emplace(std::move(parsed_msg_rv.assert_value()));
    }
    auto& msg = maybe_msg.value();
    // Find transaction handle first not to spend time on
    // message validation if we don't know anything about
    // transaction
    auto it = m_tid_to_handle.find(msg.header.transaction_id);
    if (it == m_tid_to_handle.end()) {
        m_stat.transaction_not_found.inc();
        return make_error_code(ClientError::transaction_not_found);
    }
    auto hnd = it->second;

    // Check authorization
    if (m_settings.maybe_auth.has_value()) {
        auto& auth = m_settings.maybe_auth.value();
        // We don't check username because per:
        // RFC5389: 10.1.2.  Receiving a Request or Indication:
        // The response MUST NOT contain the USERNAME attribute.
        auto is_valid_rv = msg.is_valid(view, auth.integrity);
        if (is_valid_rv.is_error()) {
            return is_valid_rv.assert_error();
        }

        if (!is_valid_rv.assert_value().has_value()) {
            if (!m_settings.allow_unauthenticated_alternate || !msg.is_alternate_server()) {
                m_stat.integrity_missing.inc();
                return make_error_code(ClientError::no_integrity_attribute_in_response);
            } // otherwise process as authenticated alternate server response
        } else  if (!is_valid_rv.assert_value().value()) {
            m_stat.integrity_check_errors.inc();
            return make_error_code(ClientError::digest_is_not_valid);
        }
    }

    if (msg.header.cls == Class::success_response()) {
        return handle_success_response(now, hnd, msg);
    } else if (msg.header.cls == Class::error_response()) {
        return handle_error_response(now, hnd, msg);
    }

    return success();
}

ClientUDP::Next ClientUDP::next(Timepoint now) {
    // Progress with pending transactions
    while (!m_tid_timeline.empty() && !m_tid_timeline.top().first.is_after(now)) {
        Handle hnd = m_tid_timeline.top().second;
        m_tid_timeline.pop();
        if (auto it = m_tmap.find(hnd); it != m_tmap.end()) {
            auto& t = it->second;
            if (auto next = t.rtx_algo->next(now); next.has_value()) {
                m_stat.retransmits.inc();
                t.rtx_count++;
                m_tid_timeline.emplace(std::make_pair(*next, hnd));
                m_effects.emplace(SendData{util::ConstBinaryView(t.msg_data)});
            } else {
                auto reason = TransactionFailed::Timeout{};
                m_effects.emplace(TransactionFailed{hnd, reason});
            }
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

ReturnValue<ClientUDP::Handle> ClientUDP::do_create(Timepoint now, TransactionId&& tid, Request&& rq) {
    stun::Message request {
        stun::Header {
            stun::Class::request(),
            stun::Method::binding(),
            std::move(tid)
        },
        stun::AttributeSet::create(std::move(rq.attrs), std::move(rq.unknown_attrs)),
        stun::IsRFC3489{false}
    };

    MaybeIntegrity maybe_integrity;
    if (m_settings.maybe_auth.has_value()) {
        const auto& auth = m_settings.maybe_auth.value();
        request.attribute_set.emplace(Attribute::create(UsernameAttribute{auth.username}));
        maybe_integrity = auth.integrity;
    }

    if (m_settings.use_fingerprint) {
        request.attribute_set.emplace(Attribute::create(FingerprintAttribute{0}));
    }

    return request
        .build(maybe_integrity)
        .fmap([&](util::ByteVec&& data) {
            Handle handle = allocate_handle();
            m_tid_to_handle.emplace(request.header.transaction_id, handle);

            auto rtx_algo = allocate_rtx_algo(now);
            Transaction transaction(now,
                                    std::move(request.header.transaction_id),
                                    std::move(data),
                                    std::move(rtx_algo));
            auto [it, _] = m_tmap.emplace(handle, std::move(transaction));
            m_effects.emplace(SendData{util::ConstBinaryView(it->second.msg_data)});
            m_stat.started.inc();
            return handle;
        });

}

MaybeError ClientUDP::handle_success_response(Timepoint now, Handle hnd, Message& msg) {
    // RFC5389:
    // 7.3.3.  Processing a Success Response
    // If the success response contains unknown
    // comprehension-required attributes, the response is
    // discarded and the transaction is considered to have failed.
    auto ucr = msg.attribute_set.unknown_comprehension_required();
    if (!ucr.empty()) {
        m_stat.unknown_attribute.inc();
        auto reason = TransactionFailed::UnknownComprehensionRequiredAttribute{std::move(ucr)};
        m_effects.emplace(TransactionFailed{hnd, std::move(reason)});
        return success();
    }

    std::optional<Duration> maybe_rtt;
    if (auto it = m_tmap.find(hnd); it != m_tmap.end()) {
        auto& trans = it->second;
        if (trans.rtx_count == 0) {
            maybe_rtt = now - trans.create_time;
        }
    }

    if (const auto& maybe_xor_mapped = msg.attribute_set.xor_mapped(); maybe_xor_mapped.has_value()) {
        m_stat.success.inc();
        auto& xor_mapped = maybe_xor_mapped.value().get();
        net::UdpEndpoint ep{xor_mapped.addr.to_address(msg.header.transaction_id), xor_mapped.port};
        m_effects.emplace(TransactionOk{hnd, std::move(ep), std::move(msg), maybe_rtt});
        return success();
    }

    if (const auto& maybe_mapped = msg.attribute_set.mapped(); maybe_mapped.has_value()) {
        m_stat.success.inc();
        auto& mapped = maybe_mapped.value().get();
        net::UdpEndpoint ep{mapped.addr, mapped.port};
        m_effects.emplace(TransactionOk{hnd, std::move(ep), std::move(msg), maybe_rtt});
        return success();
    }

    m_stat.no_mapped_address.inc();
    auto reason = TransactionFailed::Error{make_error_code(ClientError::no_address_in_response)};
    m_effects.emplace(TransactionFailed{hnd, std::move(reason)});
    return success();
}

MaybeError ClientUDP::handle_error_response(Timepoint now, Handle hnd, const Message& msg) {
    // RFC5389:
    // 7.3.4.  Processing an Error Response
    // If the error response contains unknown comprehension-required
    // attributes, or if the error response does not contain an ERROR-CODE
    // attribute, then the transaction is simply considered to have failed.
    auto ucr = msg.attribute_set.unknown_comprehension_required();
    if (!ucr.empty()) {
        m_stat.unknown_attribute.inc();
        auto reason = TransactionFailed::UnknownComprehensionRequiredAttribute{std::move(ucr)};
        m_effects.emplace(TransactionFailed{hnd, std::move(reason)});
        return success();
    }

    const auto& maybe_msg_error_code = msg.attribute_set.error_code();
    if (!maybe_msg_error_code.has_value()) {
        m_stat.no_error_code.inc();
        auto reason = TransactionFailed::Error{make_error_code(ClientError::no_error_code_in_response)};
        m_effects.emplace(TransactionFailed{hnd, std::move(reason)});
        return success();
    }
    const auto& msg_error_code = maybe_msg_error_code.value().get();

    if (msg.is_alternate_server()) {
        m_stat.response_3xx.inc();
        const auto& maybe_alt_srv = msg.attribute_set.alternate_server();
        if (!maybe_alt_srv.has_value()) {
            m_stat.no_alternate_server_attr.inc();
            auto reason = TransactionFailed::Error{make_error_code(ClientError::no_alternate_server_in_response)};
            m_effects.emplace(TransactionFailed{hnd, std::move(reason)});
            return success();
        }
        const auto& alt_srv = maybe_alt_srv.value().get();
        m_stat.try_alternate_responses.inc();
        auto reason = TransactionFailed::AlternateServer{{alt_srv.addr, alt_srv.port}};
        m_effects.emplace(TransactionFailed{hnd, std::move(reason)});
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
        m_effects.emplace(TransactionFailed{hnd, std::move(reason)});
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
            if (maybe_ua.has_value()) {
                auto reason = TransactionFailed::UnknownAttributeReported{maybe_ua->get().types};
                m_effects.emplace(TransactionFailed{hnd, std::move(reason)});
                return success();
            }
        }
        auto reason = TransactionFailed::ErrorCode{msg_error_code};
        m_effects.emplace(TransactionFailed{hnd, std::move(reason)});
        return success();
    }
    case 5: {
        // If the error code is 500 through 599, the client MAY resend the
        // request; clients that do so MUST limit the number of times they do
        // this.
        m_stat.response_5xx.inc();
        if (auto it = m_tmap.find(hnd); it != m_tmap.end()) {
            auto& transaction = it->second;
            switch (transaction.rtx_algo->process_5xx(now, msg_error_code.code)) {
            case RetransmitAlgo::Process5xxResult::RetransmitScheduled:
                return success();
            case RetransmitAlgo::Process5xxResult::TransactionFailed: {
                auto reason = TransactionFailed::ErrorCode{msg_error_code};
                m_effects.emplace(TransactionFailed{hnd, std::move(reason)});
                return success();
            }
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

ClientUDP::RetransmitAlgoPtr ClientUDP::allocate_rtx_algo(Timepoint now) {
    return
        std::visit(
            util::overloaded {
                [&](const Settings::RetransmitDefault& settings) {
                    return std::make_unique<RetransmitAlgo>(settings, now);
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

ClientUDP::Transaction::Transaction(Timepoint now, TransactionId&& t, util::ByteVec&& data, RetransmitAlgoPtr&& algo)
    : tid(std::move(t))
    , msg_data(std::move(data))
    , rtx_algo(std::move(algo))
    , create_time(now)
{}

ClientUDP::Transaction::~Transaction()
{}

ClientUDP::RetransmitAlgo::RetransmitAlgo(const Settings::RetransmitDefault& settings, Timepoint now)
    : m_settings(settings)
    , m_next_rtx(now.advance(settings.initial_rto))
{}

ClientUDP::MaybeTimepoint ClientUDP::RetransmitAlgo::next(Timepoint) {
    return {};
}

ClientUDP::RetransmitAlgo::Process5xxResult
ClientUDP::RetransmitAlgo::process_5xx(Timepoint, int) {
    return Process5xxResult::TransactionFailed;
}

}

