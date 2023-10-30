//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN server tests
//

#include <gtest/gtest.h>

#include "stun/stun_server_stateful.hpp"
#include "stun/stun_transaction_id_hash.hpp"
#include <chrono>
#include <random>

namespace freewebrtc::test {

class STUNServerStatefulTest : public ::testing::Test {
public:
    using Clock = std::chrono::steady_clock;
    using StunServer = stun::server::Stateful<Clock>;
};

// ================================================================================
// Positive cases

TEST_F(STUNServerStatefulTest, create_simple_response) {
    using namespace std::chrono_literals;
    StunServer::Settings settings{40s, stun::MurmurTransactionIdHash<std::random_device>::create()};
    StunServer server(settings);
}

}
