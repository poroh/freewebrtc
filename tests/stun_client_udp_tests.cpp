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

namespace freewebrtc::test {

using namespace std::chrono_literals;

class StunClientTest : public ::testing::Test {
public:
    StunClientTest()
        : stun_server_ipv4(net::ip::Address::from_string("192.168.0.1").assert_value())
        , stun_server_ep4(net::UdpEndpoint{stun_server_ipv4, net::Port(3478)})
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
    using ClientUDP = stun::ClientUDP;
    using Settings  = stun::ClientUDP::Settings;
    using StunServer = stun::server::Stateless;
    using Message  = stun::Message;

    const net::ip::Address stun_server_ipv4;
    const net::UdpEndpoint stun_server_ep4;
    const net::ip::Address nat_ipv4;
    const net::UdpEndpoint nat_ep4;
    const crypto::SHA1Hash::Func sha1 = crypto::openssl::sha1;
    const Settings::Auth default_auth;
    const clock::NativeDuration tick_size {1};

    StunServer server;
    std::random_device rnd;

    void initial_request_check(ClientUDP&, std::optional<Message>&, std::optional<util::ConstBinaryView>&);
    void advance_sleeps(ClientUDP&, Timepoint& now, ClientUDP::Next&);
    void tick(Timepoint& now);
    util::ByteVec server_reponse(util::ConstBinaryView req_view);
};


void StunClientTest::initial_request_check(
        ClientUDP& client,
        std::optional<Message>& msg,
        std::optional<util::ConstBinaryView>& view)
{
    auto now = Timepoint::epoch();
    // Send request / receive response
    auto hnd_rv = client.create(
        rnd,
        now,
        ClientUDP::Request{
            stun_server_ep4,
            {}
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

void StunClientTest::advance_sleeps(ClientUDP& client, Timepoint& now, ClientUDP::Next& next) {
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
    ClientUDP client({std::nullopt, Settings::UseFingerprint{false}});
    std::optional<util::ConstBinaryView> maybe_msg_view;
    std::optional<Message> maybe_msg;
    initial_request_check(client, maybe_msg, maybe_msg_view);
    ASSERT_TRUE(maybe_msg.has_value());
    const auto& msg = maybe_msg.value();
    EXPECT_TRUE(!msg.attribute_set.has_fingerprint());
}

TEST_F(StunClientTest, initial_request_check_auth_with_fingerprint) {
    ClientUDP client({default_auth});
    std::optional<util::ConstBinaryView> maybe_msg_view;
    std::optional<Message> maybe_msg;
    initial_request_check(client, maybe_msg, maybe_msg_view);
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
    ClientUDP client({default_auth, Settings::UseFingerprint{false}});
    std::optional<util::ConstBinaryView> maybe_msg_view;
    std::optional<Message> maybe_msg;
    initial_request_check(client, maybe_msg, maybe_msg_view);
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
    Settings settings{std::nullopt};
    auto rtx_settings = std::get<Settings::RetransmitDefault>(settings.retransmit);
    ClientUDP client(settings);
    auto now = Timepoint::epoch();
    /* auto hnd = */ client.create(rnd, now, ClientUDP::Request{stun_server_ep4, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Sleep>(next));
    auto sleep = std::get<ClientUDP::Sleep>(next);
    EXPECT_EQ(sleep.sleep, rtx_settings.initial_rto);
}

TEST_F(StunClientTest, initial_request_rto_set) {
    Settings settings;
    Settings::RetransmitDefault rtx_settings{1s};
    settings.retransmit = rtx_settings;
    ClientUDP client(settings);
    auto now = Timepoint::epoch();
    /* auto hnd = */ client.create(rnd, now, ClientUDP::Request{stun_server_ep4, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Sleep>(next));
    auto sleep = std::get<ClientUDP::Sleep>(next);
    EXPECT_EQ(sleep.sleep, rtx_settings.initial_rto);
}

// ================================================================================
// Request / response

TEST_F(StunClientTest, request_response_happy_path_no_auth) {
    Settings settings{std::nullopt};
    ClientUDP client(settings);
    // Create request
    auto now = Timepoint::epoch();
    auto hnd = client.create(rnd, now, ClientUDP::Request{stun_server_ep4, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data = std::get<ClientUDP::SendData>(next);
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Sleep>(next));

    // Response using StunServer
    const auto response_data = server_reponse(sent_data.message_view);

    // Process response by Client
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data), std::nullopt).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionOk>(next));
    const auto& ok = std::get<ClientUDP::TransactionOk>(next);
    EXPECT_EQ(ok.handle, hnd);
    advance_sleeps(client, now, next);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Idle>(next));
}

TEST_F(StunClientTest, request_response_happy_path_with_auth) {
    Settings settings{default_auth};
    ClientUDP client(settings);
    // Create request
    auto now = Timepoint::epoch();
    auto hnd = client.create(rnd, now, ClientUDP::Request{stun_server_ep4, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data = std::get<ClientUDP::SendData>(next);
    next = client.next(now);

    // Response using StunServer
    const auto response_data = server_reponse(sent_data.message_view);

    // Process response by Client
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data), std::nullopt).is_value());
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
    Settings settings{std::nullopt};
    ClientUDP client(settings);
    auto now = Timepoint::epoch();

    // First request
    auto hnd1 = client.create(rnd, now, ClientUDP::Request{stun_server_ep4, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data1 = std::get<ClientUDP::SendData>(next);
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Sleep>(next));

    tick(now);

    // Second request
    auto hnd2 = client.create(rnd, now, ClientUDP::Request{stun_server_ep4, {}}).assert_value();
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data2 = std::get<ClientUDP::SendData>(next);

    // Responses by server:
    const auto response_data1 = server_reponse(sent_data1.message_view);
    const auto response_data2 = server_reponse(sent_data2.message_view);

    // Process First response by Client
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data1), std::nullopt).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionOk>(next));
    const auto& ok1 = std::get<ClientUDP::TransactionOk>(next);
    EXPECT_EQ(ok1.handle, hnd1);

    tick(now);

    // Process Second response by Client
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data2), std::nullopt).is_value());
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
    Settings settings{std::nullopt};
    ClientUDP client(settings);
    auto now = Timepoint::epoch();

    // First request
    auto hnd1 = client.create(rnd, now, ClientUDP::Request{stun_server_ep4, {}}).assert_value();
    auto next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data1 = std::get<ClientUDP::SendData>(next);
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Sleep>(next));

    tick(now);

    // Second request
    auto hnd2 = client.create(rnd, now, ClientUDP::Request{stun_server_ep4, {}}).assert_value();
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::SendData>(next));
    auto sent_data2 = std::get<ClientUDP::SendData>(next);

    // Responses by server:
    const auto response_data1 = server_reponse(sent_data1.message_view);
    const auto response_data2 = server_reponse(sent_data2.message_view);

    // Process First response by Client
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data2), std::nullopt).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionOk>(next));
    const auto& ok2 = std::get<ClientUDP::TransactionOk>(next);
    EXPECT_EQ(ok2.handle, hnd2);

    tick(now);

    // Process Second response by Client
    ASSERT_TRUE(client.response(now, util::ConstBinaryView(response_data1), std::nullopt).is_value());
    next = client.next(now);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::TransactionOk>(next));
    const auto& ok1 = std::get<ClientUDP::TransactionOk>(next);
    EXPECT_EQ(ok1.handle, hnd1);

    // Check that finally ClientUDP is idle
    advance_sleeps(client, now, next);
    ASSERT_TRUE(std::holds_alternative<ClientUDP::Idle>(next));
}




}
