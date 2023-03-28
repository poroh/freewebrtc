//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN parse tests
//

#include <gtest/gtest.h>

#include "stun/stun_message.hpp"
#include "crypto/openssl/openssl_hash.hpp"

namespace freewebrtc::test {

class STUNMessageParserTest : public ::testing::Test {
};

// ================================================================================
// Positive cases

TEST_F(STUNMessageParserTest, rfc5796_2_1_sample_request) {
    // This request uses the following parameters:
    //
    // Software name:  "STUN test client" (without quotes)
    //
    // Username:  "evtj:h6vY" (without quotes)/
    //
    // Password:  "VOkJxbRl1RmTxUk/WvJxBt" (without quotes)
    //
    std::vector<uint8_t> request = {
          0x00, 0x01, 0x00, 0x58,  //    Request type and message length
          0x21, 0x12, 0xa4, 0x42,  //    Magic cookie
          0xb7, 0xe7, 0xa7, 0x01,  // }
          0xbc, 0x34, 0xd6, 0x86,  // }  Transaction ID
          0xfa, 0x87, 0xdf, 0xae,  // }
          0x80, 0x22, 0x00, 0x10,  //    SOFTWARE attribute header
          0x53, 0x54, 0x55, 0x4e,  // }
          0x20, 0x74, 0x65, 0x73,  // }  User-agent...
          0x74, 0x20, 0x63, 0x6c,  // }  ...name
          0x69, 0x65, 0x6e, 0x74,  // }
          0x00, 0x24, 0x00, 0x04,  //    PRIORITY attribute header
          0x6e, 0x00, 0x01, 0xff,  //    ICE priority value
          0x80, 0x29, 0x00, 0x08,  //    ICE-CONTROLLED attribute header
          0x93, 0x2f, 0xf9, 0xb1,  // }  Pseudo-random tie breaker...
          0x51, 0x26, 0x3b, 0x36,  // }   ...for ICE control
          0x00, 0x06, 0x00, 0x09,  //    USERNAME attribute header
          0x65, 0x76, 0x74, 0x6a,  // }
          0x3a, 0x68, 0x36, 0x76,  // }  Username (9 bytes) and padding (3 bytes)
          0x59, 0x20, 0x20, 0x20,  // }
          0x00, 0x08, 0x00, 0x14,  //    MESSAGE-INTEGRITY attribute header
          0x9a, 0xea, 0xa7, 0x0c,  // }
          0xbf, 0xd8, 0xcb, 0x56,  // }
          0x78, 0x1e, 0xf2, 0xb5,  // }  HMAC-SHA1 fingerprint
          0xb2, 0xd3, 0xf2, 0x49,  // }
          0xc1, 0xb5, 0x71, 0xa2,  // }
          0x80, 0x28, 0x00, 0x04,  //    FINGERPRINT attribute header
          0xe5, 0x7a, 0x3b, 0xcf   //    CRC32 fingerprint
    };
    stun::ParseStat stat;
    auto result = stun::Message::parse(util::ConstBinaryView(request), stat);
    EXPECT_EQ(stat.success.count(), 1);
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->is_rfc3489);
    EXPECT_EQ(result->header.cls, stun::Class::request());
    EXPECT_EQ(result->header.method, stun::Method::binding());
    auto password = stun::Password::short_term(precis::OpaqueString("VOkJxbRl1RmTxUk/WvJxBt"), crypto::openssl::sha1);
    ASSERT_TRUE(password.value().has_value());
    auto is_valid_result = result->is_valid(util::ConstBinaryView(request), *password.value(), crypto::openssl::sha1);
    ASSERT_TRUE(!is_valid_result.error().has_value());
    EXPECT_TRUE(is_valid_result.value().has_value() && is_valid_result.value()->get().has_value() && *is_valid_result.value()->get());
    auto username = result->attribute_set.username();
    ASSERT_TRUE(username.has_value());
    EXPECT_EQ(username->get().name.value, "evtj:h6vY");
    auto software = result->attribute_set.software();
    ASSERT_TRUE(software.has_value());
    EXPECT_EQ(software->get().name, "STUN test client");
}

TEST_F(STUNMessageParserTest, rfc5796_2_2_sample_response) {
    // 2.2.  Sample IPv4 Response
    //
    // Password:  "VOkJxbRl1RmTxUk/WvJxBt" (without quotes)
    //
    // Software name:  "test vector" (without quotes)
    //
    // Mapped address:  192.0.2.1 port 32853
    std::vector<uint8_t> response = {
        0x01, 0x01, 0x00, 0x3c,  //    Response type and message length
        0x21, 0x12, 0xa4, 0x42,  //    Magic cookie
        0xb7, 0xe7, 0xa7, 0x01,  // }
        0xbc, 0x34, 0xd6, 0x86,  // }  Transaction ID
        0xfa, 0x87, 0xdf, 0xae,  // }
        0x80, 0x22, 0x00, 0x0b,  //    SOFTWARE attribute header
        0x74, 0x65, 0x73, 0x74,  // }
        0x20, 0x76, 0x65, 0x63,  // }  UTF-8 server name
        0x74, 0x6f, 0x72, 0x20,  // }
        0x00, 0x20, 0x00, 0x08,  //    XOR-MAPPED-ADDRESS attribute header
        0x00, 0x01, 0xa1, 0x47,  //    Address family (IPv4) and xor'd mapped port number
        0xe1, 0x12, 0xa6, 0x43,  //    Xor'd mapped IPv4 address
        0x00, 0x08, 0x00, 0x14,  //    MESSAGE-INTEGRITY attribute header
        0x2b, 0x91, 0xf5, 0x99,  // }
        0xfd, 0x9e, 0x90, 0xc3,  // }
        0x8c, 0x74, 0x89, 0xf9,  // }  HMAC-SHA1 fingerprint
        0x2a, 0xf9, 0xba, 0x53,  // }
        0xf0, 0x6b, 0xe7, 0xd7,  // }
        0x80, 0x28, 0x00, 0x04,  //    FINGERPRINT attribute header
        0xc0, 0x7d, 0x4c, 0x96,  //    CRC32 fingerprint
    };

    stun::ParseStat stat;
    auto result = stun::Message::parse(util::ConstBinaryView(response), stat);
    EXPECT_EQ(stat.success.count(), 1);
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->is_rfc3489);
    EXPECT_EQ(result->header.cls, stun::Class::success_response());
    EXPECT_EQ(result->header.method, stun::Method::binding());

    auto password = stun::Password::short_term(precis::OpaqueString("VOkJxbRl1RmTxUk/WvJxBt"), crypto::openssl::sha1);
    ASSERT_TRUE(password.value().has_value());
    auto is_valid_result = result->is_valid(util::ConstBinaryView(response), *password.value(), crypto::openssl::sha1);
    ASSERT_TRUE(!is_valid_result.error().has_value());
    EXPECT_TRUE(is_valid_result.value().has_value() && is_valid_result.value()->get().has_value() && *is_valid_result.value()->get());

    auto software = result->attribute_set.software();
    ASSERT_TRUE(software.has_value());
    EXPECT_EQ(software->get().name, "test vector");
}


// ================================================================================
// Negative cases



}

