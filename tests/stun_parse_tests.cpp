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
    ASSERT_TRUE(result.has_value());
    auto password = stun::Password::short_term(precis::OpaqueString("VOkJxbRl1RmTxUk/WvJxBt"), crypto::openssl::sha1);
    ASSERT_TRUE(password.value() != nullptr);
    auto is_valid_result = result->is_valid(util::ConstBinaryView(request), *password.value(), crypto::openssl::sha1);
    ASSERT_TRUE(!is_valid_result.error().has_value());
    EXPECT_TRUE(is_valid_result.value() && is_valid_result.value()->has_value() && **is_valid_result.value());
    auto username = result->attribute_set.username();
    ASSERT_TRUE(username.has_value());
    EXPECT_EQ(username->get().name.value, "evtj:h6vY");
}

// ================================================================================
// Negative cases



}

