//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN server tests
//

#include <gtest/gtest.h>
#include <random>

#include "stun/stun_server_stateless.hpp"
#include "crypto/openssl/openssl_hash.hpp"

namespace freewebrtc::test {

class STUNServerStatelessTest : public ::testing::Test {
public:
    using Clock = std::chrono::steady_clock;
    using StunServer = stun::server::Stateless;
    const net::Endpoint endpoint = net::UdpEndpoint{net::ip::Address::from_string("127.0.0.1").value(), net::Port(2023)};
    void check_error_response(const stun::Message& rsp, const stun::Message& req) {
        EXPECT_EQ(rsp.header.cls, stun::Class::error_response());
        EXPECT_EQ(rsp.header.method, req.header.method);
        EXPECT_EQ(rsp.header.transaction_id, req.header.transaction_id);
    }
    void check_error_code(const stun::Message& msg, stun::ErrorCodeAttribute::Code expected) {
        const auto maybe_errcode_attr = msg.attribute_set.error_code();
        ASSERT_TRUE(maybe_errcode_attr.has_value());
        const auto& error_code_attr = maybe_errcode_attr.value().get();
        EXPECT_EQ(error_code_attr.code, expected);
    }
    stun::TransactionId rand_tid() {
        std::random_device random;
        return stun::TransactionId::generate(random);
    }
    util::ByteVec build(const stun::Message& msg) {
        const auto rv = msg.build();
        assert(!rv.error().has_value());
        return rv.value()->get();
    }
    const crypto::SHA1Hash::Func sha1 = crypto::openssl::sha1;
};

// ================================================================================
// Positive cases


// ================================================================================
// Negative cases

// Create of 420 response code with UNKNOWN-ATTRIBUTES attribute
TEST_F(STUNServerStatelessTest, request_with_unknown_attribute_requires_comprehension) {
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
        stun::IsRFC3489{false}
    };
    const auto maybe_rsp = server.process(endpoint, util::ConstBinaryView(build(request)));
    ASSERT_TRUE(maybe_rsp.has_value());

    const auto& rsp = maybe_rsp.value();
    check_error_response(rsp, request);
    check_error_code(rsp, stun::ErrorCodeAttribute::UnknownAttribute);

    // Check UNKNOWN-ATTRIBUTS
    const auto maybe_unknown_attributes_attr = rsp.attribute_set.unknown_attributes();
    ASSERT_TRUE(maybe_unknown_attributes_attr.has_value());
    const auto& unknown_attributes_attr = maybe_unknown_attributes_attr.value().get();
    EXPECT_EQ(unknown_attributes_attr.types.size(), 1);
    EXPECT_EQ(unknown_attributes_attr.types[0], unknown_attr.type);
}

TEST_F(STUNServerStatelessTest, request_with_username_attribute_without_integrity) {
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
        stun::IsRFC3489{false}
    };
    const auto maybe_rsp = server.process(endpoint, util::ConstBinaryView(build(request)));
    ASSERT_TRUE(maybe_rsp.has_value());

    // General response checks:
    const auto& rsp = maybe_rsp.value();
    check_error_response(rsp, request);
    check_error_code(rsp, stun::ErrorCodeAttribute::BadRequest);
}

TEST_F(STUNServerStatelessTest, request_with_integrity_attribute_without_username) {
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
        stun::IsRFC3489{false}
    };

    auto maybepassword = stun::Password::short_term(precis::OpaqueString("1234"), sha1);
    ASSERT_TRUE(maybepassword.value().has_value());
    auto password = maybepassword.value()->get();
    const auto rv = request.build(stun::IntegrityData{password, sha1});
    const auto request_data = rv.value()->get();
    const auto maybe_rsp = server.process(endpoint, util::ConstBinaryView(request_data));
    ASSERT_TRUE(maybe_rsp.has_value());

    // General response checks:
    const auto& rsp = maybe_rsp.value();
    check_error_response(rsp, request);
    check_error_code(rsp, stun::ErrorCodeAttribute::BadRequest);
}

}
