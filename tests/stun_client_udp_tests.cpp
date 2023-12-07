//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN client tests
//

#include <gtest/gtest.h>
#include "stun/stun_client_udp.hpp"
#include "crypto/openssl/openssl_hash.hpp"
#include "stun/stun_server_stateless.hpp"
#include "stun/stun_error.hpp"

namespace freewebrtc::test {

using namespace std::chrono_literals;

class StunClientTest : public ::testing::Test {
public:
    StunClientTest()
        : local_ipv4(net::ip::Address::from_string("192.168.0.1").assert_value())
        , stun_server_ipv4(net::ip::Address::from_string("192.168.0.2").assert_value())
        , stun_server_ipv4_2(net::ip::Address::from_string("192.168.0.3").assert_value())
        , nat_ipv4(net::ip::Address::from_string("10.0.0.1").assert_value())
        , nat_ep4(net::UdpEndpoint{nat_ipv4, net::Port(3478)})
        , default_auth{
                precis::OpaqueString{"john doe"},
                stun::IntegrityData{
                    stun::Password::short_term(precis::OpaqueString("1234"), sha1).assert_value(),
                    sha1
                }
            }
        , server(default_auth.integrity.hash)

    {
        server.add_user(default_auth.username, default_auth.integrity.password);
    }
    using Timepoint = clock::Timepoint;
    using Duration  = clock::NativeDuration;
    using ClientUDP = stun::ClientUDP;
    using Settings  = stun::ClientUDP::Settings;
    using StunServer = stun::server::Stateless;
    using Message  = stun::Message;

    const net::ip::Address local_ipv4;
    const net::ip::Address stun_server_ipv4;
    const net::ip::Address stun_server_ipv4_2;
    const net::ip::Address nat_ipv4;
    const net::UdpEndpoint nat_ep4;
    const crypto::SHA1Hash::Func sha1 = crypto::openssl::sha1;
    const ClientUDP::Auth default_auth;
    const Duration tick_size {1};

    StunServer server;
    std::random_device rnd;

    void initial_request_check(
        ClientUDP&,
        std::optional<Message>&,
        std::optional<util::ConstBinaryView>&,
        const ClientUDP::MaybeAuth& maybe_auth = std::nullopt);
    void advance_sleeps(ClientUDP&, Timepoint& now, ClientUDP::Effect&);
    void tick(Timepoint& now);
    util::ByteVec server_reponse(util::ConstBinaryView req_view);
};


void StunClientTest::initial_request_check(
        ClientUDP& client,
        std::optional<Message>& msg,
        std::optional<util::ConstBinaryView>& view,
        const ClientUDP::MaybeAuth& maybe_auth)
{
    auto now = Timepoint::epoch();
    // Send request / receive response
    auto hnd_rv = client.create(
        rnd,
        now,
        ClientUDP::Request{
            .path = {local_ipv4, stun_server_ipv4},
            .maybe_auth = maybe_auth
        });
    ASSERT_TRUE(hnd_rv.is_value());
    const auto handle = hnd_rv.assert_value();
    const auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    const auto send = std::get<ClientUDP::SendData>(next);
    EXPECT_EQ(send.handle, handle);
    stun::ParseStat stat;
    const auto msg_rv = Message::parse(send.message_view, stat);
    ASSERT_TRUE(msg_rv.is_value());
    msg = msg_rv.assert_value();
    view = send.message_view;
    EXPECT_EQ(msg->header.cls, stun::Class::request());
    EXPECT_EQ(msg->header.method, stun::Method::binding());
}

void StunClientTest::advance_sleeps(ClientUDP& client, Timepoint& now, ClientUDP::Effect& next) {
    while (true) {
        next = client.next(now);
        if (!std::holds_alternative<ClientUDP::Sleep>(next)) {
            break;
        }
        auto sleep = std::get<ClientUDP::Sleep>(next);
        now = now.advance(sleep.sleep);
    }
}

void StunClientTest::tick(Timepoint& now) {
    now = now.advance(tick_size);
}

util::ByteVec StunClientTest::server_reponse(util::ConstBinaryView req_view) {
    const auto r = server.process(nat_ep4, req_view);
    const auto respond = std::get<StunServer::Respond>(r);
    return respond.response.build(respond.maybe_integrity).assert_value();
}

// ================================================================================
// Initial requests checks

TEST_F(StunClientTest, initial_request_check_no_auth) {
    ClientUDP client({});
    std::optional<util::ConstBinaryView> maybe_msg_view;
    std::optional<Message> maybe_msg;
    initial_request_check(client, maybe_msg, maybe_msg_view);
    ASSERT_TRUE(maybe_msg.has_value());
    const auto& msg = maybe_msg.value();
    EXPECT_TRUE(msg.attribute_set.has_fingerprint());
}

TEST_F(StunClientTest, initial_request_check_no_auth_no_fingerprint) {
    ClientUDP client({Settings::UseFingerprint{false}});
    std::optional<util::ConstBinaryView> maybe_msg_view;
    std::optional<Message> maybe_msg;
    initial_request_check(client, maybe_msg, maybe_msg_view);
    ASSERT_TRUE(maybe_msg.has_value());
    const auto& msg = maybe_msg.value();
    EXPECT_TRUE(!msg.attribute_set.has_fingerprint());
}

TEST_F(StunClientTest, initial_request_check_auth_with_fingerprint) {
    ClientUDP client({});
    std::optional<util::ConstBinaryView> maybe_msg_view;
    std::optional<Message> maybe_msg;
    initial_request_check(client, maybe_msg, maybe_msg_view, default_auth);
    ASSERT_TRUE(maybe_msg.has_value() && maybe_msg_view.has_value());
    const auto& msg_view = maybe_msg_view.value();
    const auto& msg = maybe_msg.value();
    EXPECT_TRUE(msg.attribute_set.has_fingerprint());
    auto is_valid_rv = msg.is_valid(msg_view, default_auth.integrity);
    ASSERT_TRUE(is_valid_rv.is_value());
    ASSERT_TRUE(is_valid_rv.assert_value().has_value());
    EXPECT_TRUE(is_valid_rv.assert_value().value());
}

TEST_F(StunClientTest, initial_request_check_auth_no_fingerprint) {
    ClientUDP client({Settings::UseFingerprint{false}});
    std::optional<util::ConstBinaryView> maybe_msg_view;
    std::optional<Message> maybe_msg;
    initial_request_check(client, maybe_msg, maybe_msg_view, default_auth);
    ASSERT_TRUE(maybe_msg.has_value() && maybe_msg_view.has_value());
    const auto& msg_view = maybe_msg_view.value();
    const auto& msg = maybe_msg.value();
    EXPECT_TRUE(!msg.attribute_set.has_fingerprint());
    auto is_valid_rv = msg.is_valid(msg_view, default_auth.integrity);
    ASSERT_TRUE(is_valid_rv.is_value());
    ASSERT_TRUE(is_valid_rv.assert_value().has_value());
    EXPECT_TRUE(is_valid_rv.assert_value().value());
}

TEST_F(StunClientTest, initial_request_rto_default) {
    Settings settings;
    ClientUDP client(settings);
    auto now = Timepoint::epoch();
    /* auto hnd = */ client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Sleep>(next));
    auto sleep = std::get<ClientUDP::Sleep>(next);
    EXPECT_EQ(sleep.sleep, settings.rto_settings.initial_rto);
}

TEST_F(StunClientTest, initial_request_rto_set) {
    Settings settings;
    Settings::RetransmitDefault rtx_settings{1s};
    settings.retransmit = rtx_settings;
    ClientUDP client(settings);
    auto now = Timepoint::epoch();
    /* auto hnd = */ client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Sleep>(next));
    auto sleep = std::get<ClientUDP::Sleep>(next);
    EXPECT_EQ(sleep.sleep, settings.rto_settings.initial_rto);
}

// ================================================================================
// Request / response

TEST_F(StunClientTest, request_response_happy_path_no_auth) {
    Settings settings;
    ClientUDP client(settings);
    // Create request
    auto now = Timepoint::epoch();
    auto hnd = client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data = std::get<ClientUDP::SendData>(next);
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Sleep>(next));

    // Response using StunServer
    const auto response_data = server_reponse(sent_data.message_view);

    // Process response by Client
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data)).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionOk>(next));
    const auto& ok = std::get<ClientUDP::TransactionOk>(next);
    EXPECT_EQ(ok.handle, hnd);
    advance_sleeps(client, now, next);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Idle>(next));
}

TEST_F(StunClientTest, request_response_happy_path_with_auth) {
    Settings settings;
    ClientUDP client(settings);
    // Create request
    auto now = Timepoint::epoch();
    auto hnd = client.create(
        rnd,
        now,
        ClientUDP::Request{
            .path = {local_ipv4, stun_server_ipv4},
            .maybe_auth = default_auth
        }).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data = std::get<ClientUDP::SendData>(next);
    next = client.next(now);

    // Response using StunServer
    const auto response_data = server_reponse(sent_data.message_view);

    // Process response by Client
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data)).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionOk>(next));
    const auto& ok = std::get<ClientUDP::TransactionOk>(next);
    EXPECT_EQ(ok.handle, hnd);
    advance_sleeps(client, now, next);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Idle>(next));
}

TEST_F(StunClientTest, request_response_parallel_transactions_abab) {
    // Idea of the test that UDP client is requested to create second
    // transaction when the first transaction is not responded.
    // then responses are received in direct order
    Settings settings;
    ClientUDP client(settings);
    auto now = Timepoint::epoch();

    // First request
    auto hnd1 = client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data1 = std::get<ClientUDP::SendData>(next);
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Sleep>(next));

    tick(now);

    // Second request
    auto hnd2 = client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data2 = std::get<ClientUDP::SendData>(next);

    // Responses by server:
    const auto response_data1 = server_reponse(sent_data1.message_view);
    const auto response_data2 = server_reponse(sent_data2.message_view);

    // Process First response by Client
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data1)).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionOk>(next));
    const auto& ok1 = std::get<ClientUDP::TransactionOk>(next);
    EXPECT_EQ(ok1.handle, hnd1);

    tick(now);

    // Process Second response by Client
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data2)).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionOk>(next));
    const auto& ok2 = std::get<ClientUDP::TransactionOk>(next);
    EXPECT_EQ(ok2.handle, hnd2);

    // Check that finally ClientUDP is idle
    advance_sleeps(client, now, next);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Idle>(next));
}

TEST_F(StunClientTest, request_response_parallel_transactions_abba) {
    // Idea of the test that UDP client is requested to create second
    // transaction when the first transaction is not responded.
    // then responses are received in direct order
    Settings settings;
    ClientUDP client(settings);
    auto now = Timepoint::epoch();

    // First request
    auto hnd1 = client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data1 = std::get<ClientUDP::SendData>(next);
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Sleep>(next));

    tick(now);

    // Second request
    auto hnd2 = client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data2 = std::get<ClientUDP::SendData>(next);

    // Responses by server:
    const auto response_data1 = server_reponse(sent_data1.message_view);
    const auto response_data2 = server_reponse(sent_data2.message_view);

    // Process First response by Client
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data2)).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionOk>(next));
    const auto& ok2 = std::get<ClientUDP::TransactionOk>(next);
    EXPECT_EQ(ok2.handle, hnd2);

    tick(now);

    // Process Second response by Client
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data1)).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionOk>(next));
    const auto& ok1 = std::get<ClientUDP::TransactionOk>(next);
    EXPECT_EQ(ok1.handle, hnd1);

    // Check that finally ClientUDP is idle
    advance_sleeps(client, now, next);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Idle>(next));
}

// ================================================================================
// Retransmits

TEST_F(StunClientTest, retransmits) {
    Settings settings;
    ClientUDP client(settings);
    auto rtx_settings = std::get<Settings::RetransmitDefault>(settings.retransmit);
    auto now = Timepoint::epoch();
    auto hnd = client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto send_time = now;
    advance_sleeps(client, now, next);

    unsigned backoff = 1;
    for (unsigned i = 0; i + 1 < rtx_settings.request_count; ++i) {
        auto expected_timeout = settings.rto_settings.initial_rto * backoff;
        if (rtx_settings.max_rto.has_value()) {
            expected_timeout = std::max(expected_timeout, rtx_settings.max_rto.value());
        }
        EXPECT_EQ(now - send_time, expected_timeout);
        ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
        send_time = now;
        backoff = backoff * 2;
        advance_sleeps(client, now, next);
    }

    // RFC5389:
    // 7.2.1.  Sending over UDP:
    // If, after the last request, a duration equal to Rm times the
    // RTO has passed without a response (providing ample time to get
    // a response if only this final request actually succeeds), the
    // client SHOULD consider the transaction to have failed.  Rm
    // SHOULD be configurable and SHOULD have a default of 16.
    auto expected_timeout = settings.rto_settings.initial_rto * rtx_settings.retransmission_multiplier;
    if (rtx_settings.max_rto.has_value()) {
        expected_timeout = std::max(expected_timeout, rtx_settings.max_rto.value());
    }
    EXPECT_EQ(now - send_time, expected_timeout);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionFailed>(next));
    const auto& fail = std::get<ClientUDP::TransactionFailed>(next);
    EXPECT_EQ(fail.handle, hnd);
}

TEST_F(StunClientTest, retransmits_rfc5389_example_timing_checks) {
    // Idea of the test is to check example of timingsfrom RFC5389
    Settings settings;

    // Force RFC5389 settings:
    Settings::RetransmitDefault rtx_settings;
    settings.rto_settings.initial_rto = 500ms;
    rtx_settings.max_rto = std::nullopt;
    rtx_settings.request_count = 7;
    settings.retransmit = rtx_settings;

    ClientUDP client(settings);
    std::vector<Duration> send_times;
    auto now = Timepoint::epoch();
    auto start = now;
    client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    send_times.emplace_back(now - start);
    advance_sleeps(client, now, next);

    for (unsigned i = 0; i + 1 < rtx_settings.request_count; ++i) {
        ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
        send_times.emplace_back(now - start);
        advance_sleeps(client, now, next);
    }
    // For example, assuming an RTO of 500 ms, requests would be sent
    // at times 0 ms, 500 ms, 1500 ms, 3500 ms, 7500 ms, 15500 ms, and
    // 31500 ms.
    const std::vector<Duration> test_vec{0ms, 500ms, 1500ms, 3500ms, 7500ms, 15500ms, 31500ms};
    EXPECT_EQ(send_times, test_vec);

    // If the client has not received a response after 39500 ms, the
    // client will consider the transaction to have timed out.
    EXPECT_EQ(now - start, 39500ms);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionFailed>(next));
    const auto& tfailed = std::get<ClientUDP::TransactionFailed>(next);
    EXPECT_TRUE(std::holds_alternative<ClientUDP::TransactionFailed::Timeout>(tfailed.reason));
}

// ================================================================================
// Karn's algorithm and RFC629

TEST_F(StunClientTest, adjustment_of_rto_for_subsequent_request) {
    auto now = Timepoint::epoch();
    ClientUDP client({});
    // Send first request
    client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data = std::get<ClientUDP::SendData>(next);
    const auto rtt = 100ms;
    now = now.advance(rtt);
    const auto response_data = server_reponse(sent_data.message_view);
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data)).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionOk>(next));
    const auto& tok = std::get<ClientUDP::TransactionOk>(next);
    ASSERT_TRUE(tok.round_trip.has_value());
    ASSERT_EQ(tok.round_trip.value(), rtt);

    // Send second request
    auto second_rtx_start = now;
    client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    next = client.next(now);
    // Retransmit:
    EXPECT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    advance_sleeps(client, now, next);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    // rtt + K * rtt/2 where K = 4 => 100 + 200 == 300ms
    EXPECT_EQ(now - second_rtx_start, 300ms);
}

TEST_F(StunClientTest, don_adjust_on_retransmit_of_request) {
    auto now = Timepoint::epoch();
    Settings settings;
    ClientUDP client(settings);
    // Send first request
    client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    // ignore first send
    advance_sleeps(client, now, next);
    EXPECT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data = std::get<ClientUDP::SendData>(next);

    const auto rtt = 100ms;
    now = now.advance(rtt);

    const auto response_data = server_reponse(sent_data.message_view);
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data)).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionOk>(next));
    const auto& tok = std::get<ClientUDP::TransactionOk>(next);
    EXPECT_FALSE(tok.round_trip.has_value()); // No RTT value on retransmit

    // Check that subsequent request is sent within initial_rto timeout
    auto second_rtx_start = now;
    client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    next = client.next(now);
    EXPECT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    // Retransmit:
    advance_sleeps(client, now, next);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    // use backoff (2x initial RTO) for second try as timeout
    EXPECT_EQ(now - second_rtx_start, 2*settings.rto_settings.initial_rto);
}

TEST_F(StunClientTest, clear_history_after_history_duration) {
    auto now = Timepoint::epoch();
    Settings settings;
    ClientUDP client(settings);
    // Send first request
    client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data = std::get<ClientUDP::SendData>(next);
    const auto rtt = 100ms;
    now = now.advance(rtt);
    const auto response_data = server_reponse(sent_data.message_view);
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data)).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionOk>(next));
    const auto& tok = std::get<ClientUDP::TransactionOk>(next);
    ASSERT_TRUE(tok.round_trip.has_value());
    ASSERT_EQ(tok.round_trip.value(), rtt);

    // Wait history timeout
    now = now.advance(settings.rto_settings.history_duration);
    now = now.advance(1ms);
    // Create request to another destination to trigger cleanup
    client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4_2}, {}}).assert_value();
    next = client.next(now);
    auto sent_data2 = std::get<ClientUDP::SendData>(next);
    const auto response_data2 = server_reponse(sent_data2.message_view);
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data2)).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionOk>(next));

    // Send second request to the initial destination
    auto second_rtx_start = now;
    client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    next = client.next(now);
    // Retransmit:
    EXPECT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    advance_sleeps(client, now, next);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    EXPECT_EQ(now - second_rtx_start, settings.rto_settings.initial_rto);
}

// ================================================================================
// STUN error response handling

TEST_F(StunClientTest, success_response_with_unknown_comprehension_required_attribute) {
    // Test idea is to send well-formed success response but with
    // attribute that client does not understand but requires to understand.
    ClientUDP client({});
    auto now = Timepoint::epoch();
    auto hnd = client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data = std::get<ClientUDP::SendData>(next);
    stun::ParseStat parse_stat;
    auto req = Message::parse(sent_data.message_view, parse_stat).assert_value();
    auto xaddr = stun::XoredAddress::from_address(nat_ep4.address, req.header.transaction_id);
    const stun::UnknownAttribute unknown_attr1(stun::AttributeType::from_uint16(0x7fff), util::ConstBinaryView({}));
    const stun::UnknownAttribute unknown_attr2(stun::AttributeType::from_uint16(0x7ff3), util::ConstBinaryView({}));
    Message response{
        stun::Header {
            stun::Class::success_response(),
            stun::Method::binding(),
            req.header.transaction_id
        },
        stun::AttributeSet::create({
                stun::XorMappedAddressAttribute{xaddr, nat_ep4.port}
            },{
                unknown_attr1,
                unknown_attr2
            }),
        stun::IsRFC3489{false}
    };
    auto response_data = response.build().assert_value();
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data), response).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionFailed>(next));
    const auto& tfailed = std::get<ClientUDP::TransactionFailed>(next);
    EXPECT_EQ(tfailed.handle, hnd);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionFailed::UnknownComprehensionRequiredAttribute>(tfailed.reason));
    const auto& ucra = std::get<ClientUDP::TransactionFailed::UnknownComprehensionRequiredAttribute>(tfailed.reason);
    std::vector<stun::AttributeType> unknown_attrs{unknown_attr1.type, unknown_attr2.type};
    EXPECT_EQ(ucra.attrs, unknown_attrs);
}

TEST_F(StunClientTest, error_response_with_unknown_comprehension_required_attribute) {
    // Test idea is to send well-formed success response but with
    // attribute that client does not understand but requires to understand.
    ClientUDP client({});
    auto now = Timepoint::epoch();
    auto hnd = client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data = std::get<ClientUDP::SendData>(next);
    stun::ParseStat parse_stat;
    auto req = Message::parse(sent_data.message_view, parse_stat).assert_value();
    const stun::UnknownAttribute unknown_attr1(stun::AttributeType::from_uint16(0x7fff), util::ConstBinaryView({}));
    const stun::UnknownAttribute unknown_attr2(stun::AttributeType::from_uint16(0x7ff3), util::ConstBinaryView({}));
    Message response{
        stun::Header {
            stun::Class::error_response(),
            stun::Method::binding(),
            req.header.transaction_id
        },
        stun::AttributeSet::create({
                stun::ErrorCodeAttribute{stun::ErrorCodeAttribute::BadRequest, "Bad request"},
            },{
                unknown_attr1,
                unknown_attr2
            }),
        stun::IsRFC3489{false}
    };
    auto response_data = response.build().assert_value();
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data), response).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionFailed>(next));
    const auto& tfailed = std::get<ClientUDP::TransactionFailed>(next);
    EXPECT_EQ(tfailed.handle, hnd);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionFailed::UnknownComprehensionRequiredAttribute>(tfailed.reason));
    const auto& ucra = std::get<ClientUDP::TransactionFailed::UnknownComprehensionRequiredAttribute>(tfailed.reason);
    std::vector<stun::AttributeType> unknown_attrs{unknown_attr1.type, unknown_attr2.type};
    EXPECT_EQ(ucra.attrs, unknown_attrs);
}

TEST_F(StunClientTest, error_response_300_alternate_server) {
    // Test idea is to send well-formed 300 response on
    // STUN request and check that client fails transaction
    // and provides correct reason
    ClientUDP client({});
    auto now = Timepoint::epoch();
    auto hnd = client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data = std::get<ClientUDP::SendData>(next);
    stun::ParseStat parse_stat;
    auto req = Message::parse(sent_data.message_view, parse_stat).assert_value();
    const auto alternate_server_ipv4(net::ip::Address::from_string("192.168.0.2").assert_value());
    const auto alternate_server_port = net::Port{3478};
    Message response{
        stun::Header {
            stun::Class::error_response(),
            stun::Method::binding(),
            req.header.transaction_id
        },
        stun::AttributeSet::create({
            stun::ErrorCodeAttribute{stun::ErrorCodeAttribute::TryAlternate, "Try alternate server"},
            stun::AlternateServerAttribute{alternate_server_ipv4, alternate_server_port}
        }),
        stun::IsRFC3489{false}
    };
    auto response_data = response.build().assert_value();
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data), response).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionFailed>(next));
    const auto& tfailed = std::get<ClientUDP::TransactionFailed>(next);
    EXPECT_EQ(tfailed.handle, hnd);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionFailed::AlternateServer>(tfailed.reason));
    const auto& asrv = std::get<ClientUDP::TransactionFailed::AlternateServer>(tfailed.reason);
    EXPECT_EQ(asrv.server.address, alternate_server_ipv4);
    EXPECT_EQ(asrv.server.port, alternate_server_port);
}

TEST_F(StunClientTest, error_response_300_alternate_server_without_attribute) {
    // Test idea is to send well-formed 300 response on
    // STUN request and check that client fails transaction
    // and provides correct reason
    ClientUDP client({});
    auto now = Timepoint::epoch();
    auto hnd = client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data = std::get<ClientUDP::SendData>(next);
    stun::ParseStat parse_stat;
    auto req = Message::parse(sent_data.message_view, parse_stat).assert_value();
    Message response{
        stun::Header {
            stun::Class::error_response(),
            stun::Method::binding(),
            req.header.transaction_id
        },
        stun::AttributeSet::create({
            stun::ErrorCodeAttribute{stun::ErrorCodeAttribute::TryAlternate, "Try alternate server"},
        }),
        stun::IsRFC3489{false}
    };
    auto response_data = response.build().assert_value();
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data), response).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionFailed>(next));
    const auto& tfailed = std::get<ClientUDP::TransactionFailed>(next);
    EXPECT_EQ(tfailed.handle, hnd);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionFailed::Error>(tfailed.reason));
    const auto& error = std::get<ClientUDP::TransactionFailed::Error>(tfailed.reason);
    EXPECT_EQ(error.code.category(), stun::stun_client_error_category());
    EXPECT_EQ(error.code.value(), (int)stun::ClientError::no_alternate_server_in_response);
}

TEST_F(StunClientTest, error_response_420_from_server) {
    // Test idea is to send well-formed success response but with
    // attribute that client does not understand but requires to understand.
    ClientUDP client({});
    auto now = Timepoint::epoch();
    const stun::UnknownAttribute unknown_attr1(stun::AttributeType::from_uint16(0x7fff), util::ConstBinaryView({}));
    const stun::UnknownAttribute unknown_attr2(stun::AttributeType::from_uint16(0x7ff3), util::ConstBinaryView({}));
    std::vector<stun::AttributeType> unknown_attrs{unknown_attr1.type, unknown_attr2.type};
    auto hnd = client.create(rnd, now, ClientUDP::Request{{local_ipv4, stun_server_ipv4}, {}, {unknown_attr1, unknown_attr2}}).assert_value();
    auto next = client.next(now);
    auto sent_data = std::get<ClientUDP::SendData>(next);

    // Response using StunServer
    const auto response_data = server_reponse(sent_data.message_view);
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data)).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionFailed>(next));
    const auto& tfailed = std::get<ClientUDP::TransactionFailed>(next);
    EXPECT_EQ(tfailed.handle, hnd);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionFailed::UnknownAttributeReported>(tfailed.reason));
    const auto& uar = std::get<ClientUDP::TransactionFailed::UnknownAttributeReported>(tfailed.reason);
    EXPECT_EQ(uar.attrs, unknown_attrs);
}


}
