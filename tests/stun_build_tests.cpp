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
    std::optional<stun::Message> rebuild(const stun::Message& msg, const stun::MaybeIntegrity& maybe_integrity = std::nullopt) {
        const auto rv = msg.build(maybe_integrity);
        assert(!rv.error().has_value());
        const auto& data = rv.value()->get();
        stun::ParseStat stat;
        return stun::Message::parse(util::ConstBinaryView(data), stat);
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

    const auto& maybe_req = rebuild(request);
    ASSERT_TRUE(maybe_req.has_value());
    const auto& req = maybe_req.value();
    EXPECT_EQ(req.header.cls, stun::Class::request());
    EXPECT_EQ(req.header.method, stun::Method::binding());
    EXPECT_EQ(req.header.transaction_id, tid);
}

TEST_F(STUNMessageBuildTest, build_binding_request_with_fingerprint) {
    const auto tid = rand_tid();
    const stun::Message request {
        stun::Header {
            stun::Class::request(),
            stun::Method::binding(),
            tid
        },
        stun::AttributeSet::create({stun::FingerprintAttribute{0}}),
        stun::IsRFC3489{false}
    };
    const auto& maybe_req = rebuild(request);
    ASSERT_TRUE(maybe_req.has_value());
    const auto& req = maybe_req.value();
    EXPECT_EQ(req.header.cls, stun::Class::request());
    EXPECT_EQ(req.header.method, stun::Method::binding());
    EXPECT_EQ(req.header.transaction_id, tid);
}

TEST_F(STUNMessageBuildTest, build_binding_request_with_integrity) {
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

    auto password = stun::Password::short_term(precis::OpaqueString("VOkJxbRl1RmTxUk/WvJxBt"), sha1);
    ASSERT_TRUE(password.value().has_value());
    stun::IntegrityData integrity_data{*password.value(), sha1};
    const auto rv = request.build(integrity_data);
    assert(!rv.error().has_value());
    const auto& data = rv.value()->get();
    stun::ParseStat stat;
    const auto& maybe_req = stun::Message::parse(util::ConstBinaryView(data), stat);
    ASSERT_TRUE(maybe_req.has_value());
    const auto& req = maybe_req.value();
    auto is_valid_result = req.is_valid(util::ConstBinaryView(data), integrity_data);
    ASSERT_TRUE(is_valid_result.value().has_value());
    EXPECT_TRUE(is_valid_result.value().value().get());
}


}
