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

namespace freewebrtc::test {

class STUNMessageBuildTest : public ::testing::Test {
public:
    stun::TransactionId rand_tid() {
        std::random_device random;
        return stun::TransactionId::generate(random);
    }
    std::optional<stun::Message> rebuild(const stun::Message& msg) {
        const auto rv = msg.build();
        assert(!rv.error().has_value());
        const auto& data = rv.value()->get();
        stun::ParseStat stat;
        return stun::Message::parse(util::ConstBinaryView(data), stat);
    }
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

TEST_F(STUNMessageBuildTest, build_binding_request_with_finger_print) {
    const auto tid = rand_tid();
    const stun::Message request {
        stun::Header {
            stun::Class::request(),
            stun::Method::binding(),
            tid
        },
        stun::AttributeSet::create({stun::FingerprintAttribute{00}}),
        stun::IsRFC3489{false}
    };
    const auto& maybe_req = rebuild(request);
    ASSERT_TRUE(maybe_req.has_value());
    const auto& req = maybe_req.value();
    EXPECT_EQ(req.header.cls, stun::Class::request());
    EXPECT_EQ(req.header.method, stun::Method::binding());
    EXPECT_EQ(req.header.transaction_id, tid);
}


}
