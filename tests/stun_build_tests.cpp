//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN packet build tests
//

#include <gtest/gtest.h>
#include <random>

#include "stun/stun_message.hpp"
#include "crypto/openssl/openssl_hash.hpp"

namespace freewebrtc::test {

class STUNMessageBuildTest : public ::testing::Test {
public:
    stun::TransactionId rand_tid() {
        std::random_device random;
        return stun::TransactionId::generate(random);
    }
    ReturnValue<stun::Message> rebuild(const stun::Message& msg, const stun::MaybeIntegrity& maybe_integrity = std::nullopt) {
        const auto rv = msg.build(maybe_integrity);
        assert(!rv.is_error());
        const auto& data = rv.assert_value();
        stun::ParseStat stat;
        return stun::Message::parse(util::ConstBinaryView(data), stat);
    }
    void expect_headers_are_equal(const stun::Message& m1, const stun::Message& m2) {
        EXPECT_EQ(m1.header.cls,            m2.header.cls);
        EXPECT_EQ(m1.header.method,         m2.header.method);
        EXPECT_EQ(m1.header.transaction_id, m2.header.transaction_id);
    }

    const crypto::SHA1Hash::Func sha1 = crypto::openssl::sha1;
};

TEST_F(STUNMessageBuildTest, build_simple_binding_request) {
    const auto tid = rand_tid();
    const stun::Message request {
        stun::Header {
            stun::Class::request(),
            stun::Method::binding(),
            tid
        },
        stun::AttributeSet::create({}),
        stun::IsRFC3489{false}
    };

    const auto& req_rv = rebuild(request);
    ASSERT_TRUE(req_rv.is_value());
    const auto& req = req_rv.assert_value();
    expect_headers_are_equal(req, request);
}

TEST_F(STUNMessageBuildTest, build_binding_request_with_fingerprint) {
    const stun::Message request {
        stun::Header {
            stun::Class::request(),
            stun::Method::binding(),
            rand_tid()
        },
        stun::AttributeSet::create({stun::FingerprintAttribute{0}}),
        stun::IsRFC3489{false}
    };
    const auto& req_rv = rebuild(request);
    ASSERT_TRUE(req_rv.is_value());
    const auto& req = req_rv.assert_value();
    expect_headers_are_equal(req, request);
}

TEST_F(STUNMessageBuildTest, build_binding_request_with_integrity) {
    const stun::Message request {
        stun::Header {
            stun::Class::request(),
            stun::Method::binding(),
            rand_tid()
        },
        stun::AttributeSet::create({}),
        stun::IsRFC3489{false}
    };

    auto password = stun::Password::short_term(precis::OpaqueString("VOkJxbRl1RmTxUk/WvJxBt"), sha1);
    ASSERT_TRUE(password.is_value());
    stun::IntegrityData integrity_data{password.assert_value(), sha1};
    const auto rv = request.build(integrity_data);
    assert(!rv.is_error());
    const auto& data = rv.assert_value();
    stun::ParseStat stat;
    const auto& req_rv = stun::Message::parse(util::ConstBinaryView(data), stat);
    ASSERT_TRUE(req_rv.is_value());
    const auto& req = req_rv.assert_value();
    auto is_valid_rv = req.is_valid(util::ConstBinaryView(data), integrity_data);
    ASSERT_TRUE(is_valid_rv.is_value());
    EXPECT_TRUE(is_valid_rv.assert_value().value_or(false));
}

TEST_F(STUNMessageBuildTest, build_error_response_with_errocode) {
    const stun::Message response {
        stun::Header {
            stun::Class::error_response(),
            stun::Method::binding(),
            rand_tid()
        },
        stun::AttributeSet::create({
                stun::ErrorCodeAttribute{stun::ErrorCodeAttribute::BadRequest, "Bad Request"},
            }),
        stun::IsRFC3489{false}
    };
    const auto& rsp_rv = rebuild(response);
    ASSERT_TRUE(rsp_rv.is_value());
    const auto& rsp = rsp_rv.assert_value();
    expect_headers_are_equal(rsp, response);

    ASSERT_TRUE(rsp.attribute_set.error_code().has_value());
    ASSERT_TRUE(response.attribute_set.error_code().has_value());
    EXPECT_EQ(rsp.attribute_set.error_code()->get(), response.attribute_set.error_code()->get());
}

TEST_F(STUNMessageBuildTest, build_success_response_with_xor_mapped) {
    auto tid = rand_tid();
    auto xaddr = stun::XoredAddress::from_address(net::ip::Address::from_string("127.0.0.1").assert_value(), tid);
    const stun::Message response {
        stun::Header {
            stun::Class::success_response(),
            stun::Method::binding(),
            tid
        },
        stun::AttributeSet::create({
                stun::XorMappedAddressAttribute{xaddr, net::Port(1234)},
            }),
        stun::IsRFC3489{false}
    };
    const auto& rsp_rv = rebuild(response);
    ASSERT_TRUE(rsp_rv.is_value());
    const auto& rsp = rsp_rv.assert_value();
    expect_headers_are_equal(rsp, response);

    ASSERT_TRUE(rsp.attribute_set.xor_mapped().has_value());
    ASSERT_TRUE(response.attribute_set.xor_mapped().has_value());
    EXPECT_EQ(rsp.attribute_set.xor_mapped()->get(), response.attribute_set.xor_mapped()->get());
}


}
