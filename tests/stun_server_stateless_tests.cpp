//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN server tests
//

#include <gtest/gtest.h>
#include <random>
#include <chrono>

#include "stun/stun_server_stateless.hpp"
#include "crypto/openssl/openssl_hash.hpp"

namespace freewebrtc::test {

class STUNServerStatelessTest : public testing::TestWithParam<net::Endpoint> {
public:
    using Clock = std::chrono::steady_clock;
    using StunServer = stun::server::Stateless;

    void check_success_response(const stun::Message& rsp, const stun::Message& req) {
        EXPECT_EQ(rsp.header.cls, stun::Class::success_response());
        EXPECT_EQ(rsp.header.method, req.header.method);
        EXPECT_EQ(rsp.header.transaction_id, req.header.transaction_id);
    }
    void check_error_response(const stun::Message& rsp, const stun::Message& req) {
        EXPECT_EQ(rsp.header.cls, stun::Class::error_response());
        EXPECT_EQ(rsp.header.method, req.header.method);
        EXPECT_EQ(rsp.header.transaction_id, req.header.transaction_id);
    }
    void check_error_code(const stun::Message& msg, stun::ErrorCodeAttribute::Code expected) {
        const auto maybe_errcode_attr = msg.attribute_set.error_code();
        ASSERT_TRUE(maybe_errcode_attr.is_some());
        const auto& error_code_attr = maybe_errcode_attr.unwrap().get();
        EXPECT_EQ(error_code_attr.code, expected);
    }
    stun::TransactionId rand_tid() {
        std::random_device random;
        return stun::TransactionId::generate(random);
    }
    stun::TransactionId rand_tid_rfc3489() {
        std::random_device random;
        return stun::TransactionId::generate_rfc3489(random);
    }
    util::ByteVec build(const stun::Message& msg) {
        const auto rv = msg.build();
        assert(!rv.is_err());
        return rv.unwrap();
    }
    const crypto::SHA1Hash::Func sha1 = crypto::openssl::sha1;
};

const auto all_endpoints =
    testing::Values(
        net::UdpEndpoint{net::ip::Address::from_string("127.0.0.1").unwrap(), net::Port(2023)},
        net::UdpEndpoint{net::ip::Address::from_string("::1").unwrap(), net::Port(2023)}
    );

// ================================================================================
// Positive cases

TEST_P(STUNServerStatelessTest, request_rfc5389) {
    const auto endpoint = GetParam();
    StunServer server(sha1);
    const stun::Message request {
        stun::Header {
            stun::Class::request(),
            stun::Method::binding(),
            rand_tid()
        },
        stun::AttributeSet::create({}),
        stun::IsRFC3489{false},
        none()
    };
    const auto r = server.process(endpoint, util::ConstBinaryView(build(request)));
    ASSERT_TRUE(std::holds_alternative<StunServer::Respond>(r));
    const auto& rsp = std::get<StunServer::Respond>(r).response;
    check_success_response(rsp, request);
    const auto& maybe_xor_mapped = rsp.attribute_set.xor_mapped();
    ASSERT_TRUE(maybe_xor_mapped.is_some());
    const auto& xor_mapped = maybe_xor_mapped.unwrap().get();
    EXPECT_EQ(xor_mapped.addr.to_address(rsp.header.transaction_id), endpoint.address());
    EXPECT_EQ(xor_mapped.port, endpoint.port());
    // No integrity because not username / integrity in the request
    EXPECT_TRUE(!rsp.attribute_set.integrity().is_some());
}

TEST_P(STUNServerStatelessTest, request_rfc5389_authenticated) {
    const auto endpoint = GetParam();
    StunServer server(sha1);

    precis::OpaqueString joe{"joe"};
    auto joe_password_rv = stun::Password::short_term(precis::OpaqueString("1234"), sha1);
    ASSERT_TRUE(joe_password_rv.is_ok());
    auto joe_password = joe_password_rv.unwrap();
    server.add_user(joe, joe_password);

    const stun::Message request {
        stun::Header {
            stun::Class::request(),
            stun::Method::binding(),
            rand_tid()
        },
        stun::AttributeSet::create({
                stun::UsernameAttribute{joe},
                stun::FingerprintAttribute{0},
            }),
        stun::IsRFC3489{false},
        none()
    };

    stun::IntegrityData integrity_data{joe_password, sha1};
    const auto rv = request.build(integrity_data);
    ASSERT_TRUE(rv.is_ok());
    const auto request_data = rv.unwrap();
    const auto r = server.process(endpoint, util::ConstBinaryView(request_data));

    ASSERT_TRUE(std::holds_alternative<StunServer::Respond>(r));
    const auto& rsp = std::get<StunServer::Respond>(r).response;
    check_success_response(rsp, request);
    const auto& maybe_xor_mapped = rsp.attribute_set.xor_mapped();
    ASSERT_TRUE(maybe_xor_mapped.is_some());
    const auto& xor_mapped = maybe_xor_mapped.unwrap().get();
    EXPECT_EQ(xor_mapped.addr.to_address(rsp.header.transaction_id), endpoint.address());
    EXPECT_EQ(xor_mapped.port, endpoint.port());

    // Integrity must be used for response:
    const auto maybe_rsp_integrity_data = std::get<StunServer::Respond>(r).maybe_integrity;
    ASSERT_TRUE(maybe_rsp_integrity_data.is_some());
    const auto& rsp_integrity_data = maybe_rsp_integrity_data.unwrap();
    ASSERT_EQ(rsp_integrity_data.password, joe_password);

    // RFC5389: 10.1.2. Receiving a Request or Indication
    // The response MUST NOT contain the USERNAME attribute
    EXPECT_FALSE(rsp.attribute_set.username().is_some());
}

TEST_P(STUNServerStatelessTest, request_rfc3489) {
    const auto endpoint = GetParam();
    StunServer server(sha1);
    const stun::Message request {
        stun::Header {
            stun::Class::request(),
            stun::Method::binding(),
            rand_tid_rfc3489()
        },
        stun::AttributeSet::create({}),
        stun::IsRFC3489{true},
        none()
    };

    const auto r = server.process(endpoint, util::ConstBinaryView(build(request)));
    ASSERT_TRUE(std::holds_alternative<StunServer::Respond>(r));
    const auto& rsp = std::get<StunServer::Respond>(r).response;
    check_success_response(rsp, request);

    const auto& maybe_xor_mapped = rsp.attribute_set.xor_mapped();
    ASSERT_FALSE(maybe_xor_mapped.is_some());

    const auto& maybe_mapped = rsp.attribute_set.mapped();
    ASSERT_TRUE(maybe_mapped.is_some());
    const auto& mapped = maybe_mapped.unwrap().get();
    EXPECT_EQ(mapped.addr, endpoint.address());
    EXPECT_EQ(mapped.port, endpoint.port());
}

// ================================================================================
// Negative cases

// Create of 420 response code with UNKNOWN-ATTRIBUTES attribute
TEST_P(STUNServerStatelessTest, request_with_unknown_attribute_requires_comprehension) {
    const auto endpoint = GetParam();
    // RFC5389: 7.3.1.  Processing a Request
    // If the request contains one or more unknown comprehension-required
    // attributes, the server replies with an error response with an error
    // code of 420 (Unknown Attribute), and includes an UNKNOWN-ATTRIBUTES
    // attribute in the response that lists the unknown comprehension-
    // required attributes.
    StunServer server(sha1);
    const stun::UnknownAttribute unknown_attr(stun::AttributeType::from_uint16(0x7fff), util::ConstBinaryView({}));
    const stun::Message request {
        stun::Header {
            stun::Class::request(),
            stun::Method::binding(),
            rand_tid()
        },
        stun::AttributeSet::create({}, {unknown_attr}),
        stun::IsRFC3489{false},
        none()
    };
    const auto r = server.process(endpoint, util::ConstBinaryView(build(request)));
    ASSERT_TRUE(std::holds_alternative<StunServer::Respond>(r));
    const auto& rsp = std::get<StunServer::Respond>(r).response;
    check_error_response(rsp, request);
    check_error_code(rsp, stun::ErrorCodeAttribute::UnknownAttribute);

    // Check UNKNOWN-ATTRIBUTS
    const auto maybe_unknown_attributes_attr = rsp.attribute_set.unknown_attributes();
    ASSERT_TRUE(maybe_unknown_attributes_attr.is_some());
    const auto& unknown_attributes_attr = maybe_unknown_attributes_attr.unwrap().get();
    EXPECT_EQ(unknown_attributes_attr.types.size(), 1);
    EXPECT_EQ(unknown_attributes_attr.types[0], unknown_attr.type);
}

TEST_P(STUNServerStatelessTest, request_with_username_attribute_without_integrity) {
    const auto endpoint = GetParam();
    // RFC5389: 10.1.2.  Receiving a Request or Indication
    // o  If the message does not contain both a MESSAGE-INTEGRITY and a
    //   USERNAME attribute:
    //
    //   *  If the message is a request, the server MUST reject the request
    //      with an error response.  This response MUST use an error code
    //      of 400 (Bad Request).
    StunServer server(sha1);
    const stun::Message request {
        stun::Header {
            stun::Class::request(),
            stun::Method::binding(),
            rand_tid()
        },
        stun::AttributeSet::create({
                stun::UsernameAttribute{precis::OpaqueString{"test"}}
            }),
        stun::IsRFC3489{false},
        none()
    };
    const auto r = server.process(endpoint, util::ConstBinaryView(build(request)));
    ASSERT_TRUE(std::holds_alternative<StunServer::Respond>(r));
    const auto& rsp = std::get<StunServer::Respond>(r).response;

    // General response checks:
    check_error_response(rsp, request);
    check_error_code(rsp, stun::ErrorCodeAttribute::BadRequest);
}

TEST_P(STUNServerStatelessTest, request_with_integrity_attribute_without_username) {
    const auto endpoint = GetParam();
    // RFC5389: 10.1.2.  Receiving a Request or Indication
    // o  If the message does not contain both a MESSAGE-INTEGRITY and a
    //   USERNAME attribute:
    //
    //   *  If the message is a request, the server MUST reject the request
    //      with an error response.  This response MUST use an error code
    //      of 400 (Bad Request).
    StunServer server(sha1);
    const stun::Message request {
        stun::Header {
            stun::Class::request(),
            stun::Method::binding(),
            rand_tid()
        },
        stun::AttributeSet::create({}),
        stun::IsRFC3489{false},
        none()
    };

    auto maybepassword = stun::Password::short_term(precis::OpaqueString("1234"), sha1);
    ASSERT_TRUE(maybepassword.is_ok());
    auto password = maybepassword.unwrap();
    const auto rv = request.build(stun::IntegrityData{password, sha1});
    const auto request_data = rv.unwrap();
    const auto r = server.process(endpoint, util::ConstBinaryView(request_data));
    ASSERT_TRUE(std::holds_alternative<StunServer::Respond>(r));
    const auto& rsp = std::get<StunServer::Respond>(r).response;
    check_error_response(rsp, request);
    check_error_code(rsp, stun::ErrorCodeAttribute::BadRequest);
}

TEST_P(STUNServerStatelessTest, unknown_username) {
    const auto endpoint = GetParam();
    StunServer server(sha1);
    const stun::Message request {
        stun::Header {
            stun::Class::request(),
            stun::Method::binding(),
            rand_tid()
        },
        stun::AttributeSet::create({
                stun::UsernameAttribute{precis::OpaqueString{"joe"}}
            }),
        stun::IsRFC3489{false},
        none()
    };

    auto maybepassword = stun::Password::short_term(precis::OpaqueString("1234"), sha1);
    ASSERT_TRUE(maybepassword.is_ok());
    auto password = maybepassword.unwrap();
    const auto rv = request.build(stun::IntegrityData{password, sha1});
    const auto request_data = rv.unwrap();
    const auto r = server.process(endpoint, util::ConstBinaryView(request_data));
    ASSERT_TRUE(std::holds_alternative<StunServer::Respond>(r));
    const auto& rsp = std::get<StunServer::Respond>(r).response;
    check_error_response(rsp, request);
    check_error_code(rsp, stun::ErrorCodeAttribute::Unauthorized);
}

TEST_P(STUNServerStatelessTest, wrong_password) {
    const auto endpoint = GetParam();
    StunServer server(sha1);
    precis::OpaqueString joe{"joe"};
    const stun::Message request {
        stun::Header {
            stun::Class::request(),
            stun::Method::binding(),
            rand_tid()
        },
        stun::AttributeSet::create({
                stun::UsernameAttribute{joe}
            }),
        stun::IsRFC3489{false},
        none()
    };

    auto joe_password_rv = stun::Password::short_term(precis::OpaqueString("4321"), sha1);
    ASSERT_TRUE(joe_password_rv.is_ok());
    auto joe_password = joe_password_rv.unwrap();
    server.add_user(joe, joe_password);

    auto wrong_password_rv = stun::Password::short_term(precis::OpaqueString("1234"), sha1);
    ASSERT_TRUE(wrong_password_rv.is_ok());
    auto wrong_password = wrong_password_rv.unwrap();
    const auto rv = request.build(stun::IntegrityData{wrong_password, sha1});
    ASSERT_TRUE(rv.is_ok());
    const auto request_data = rv.unwrap();
    const auto r = server.process(endpoint, util::ConstBinaryView(request_data));
    ASSERT_TRUE(std::holds_alternative<StunServer::Respond>(r));
    const auto& rsp = std::get<StunServer::Respond>(r).response;
    check_error_response(rsp, request);
    check_error_code(rsp, stun::ErrorCodeAttribute::Unauthorized);
}

INSTANTIATE_TEST_SUITE_P(
    CheckAllEndpoints,
    STUNServerStatelessTest,
    all_endpoints,
    [](const auto& v) -> std::string { // More nicer test naming
        const auto& ep = v.param;
        auto index_str = std::to_string(v.index);
        return std::visit(
            util::overloaded {
                [&](const net::ip::AddressV4&) { return "IPv4_" + index_str; },
                    [&](const net::ip::AddressV6&) { return "IPv6_" + index_str; },
                    },
            ep.address().value());
    });
}
