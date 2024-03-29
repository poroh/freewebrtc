//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Network port tests
//

#include <gtest/gtest.h>
#include "net/net_port.hpp"

namespace freewebrtc::tests {

class NetPortTest : public ::testing::Test {
public:
};

TEST_F(NetPortTest, from_string_tests) {
    EXPECT_FALSE(net::Port::from_string("").is_ok());
    EXPECT_TRUE(net::Port::from_string("0").is_ok());
    EXPECT_TRUE(net::Port::from_string("1").is_ok());
    EXPECT_TRUE(net::Port::from_string("65535").is_ok());
    EXPECT_FALSE(net::Port::from_string("65536").is_ok());

    EXPECT_TRUE(net::Port::from_string("90").is_ok());
    EXPECT_EQ(net::Port::from_string("1").unwrap(), net::Port::from_uint16(1));
    EXPECT_EQ(net::Port::from_string("65535").unwrap(), net::Port::from_uint16(65535));
}

}
