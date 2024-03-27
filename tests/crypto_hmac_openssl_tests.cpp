//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// HMAC using OpenSSL tests
//

#include <gtest/gtest.h>

#include "crypto/crypto_hmac.hpp"
#include "crypto/openssl/openssl_hash.hpp"

namespace freewebrtc::test {

class CryptoHMACOpenSSLTests : public ::testing::Test {
};


TEST_F(CryptoHMACOpenSSLTests, rfc2104_test_vectors_1) {
    // key =         0x0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b
    // key_len =     16 bytes
    // data =        "Hi There"
    // data_len =    8  bytes
    // digest =      0x9294727a3638bb1c13f48ef8158bfc9d
    const auto key = std::vector<uint8_t>(16, 0x0b);
    const auto ipad = crypto::hmac::IPadKey::from_key(util::ConstBinaryView(key), crypto::openssl::md5);
    const auto opad = crypto::hmac::OPadKey::from_key(util::ConstBinaryView(key), crypto::openssl::md5);
    ASSERT_TRUE(ipad.is_ok());
    ASSERT_TRUE(opad.is_ok());
    const std::string data = "Hi There";
    const auto digest = crypto::hmac::digest({util::ConstBinaryView(data.c_str(), data.length())},
                                             opad.unwrap(),
                                             ipad.unwrap(),
                                             crypto::MD5Hash::Func{&crypto::openssl::md5});
    ASSERT_TRUE(digest.is_ok());
    crypto::MD5Hash::Value expected_v = {0x92, 0x94, 0x72, 0x7a, 0x36, 0x38, 0xbb, 0x1c, 0x13, 0xf4, 0x8e, 0xf8, 0x15, 0x8b, 0xfc, 0x9d};
    EXPECT_TRUE(crypto::MD5Hash(std::move(expected_v)) == digest.unwrap().value);
}

TEST_F(CryptoHMACOpenSSLTests, rfc2104_test_vectors_2) {
    // key =         "Jefe"
    // data =        "what do ya want for nothing?"
    // data_len =    28 bytes
    // digest =      0x750c783e6ab0b503eaa86e310a5db738
    const std::string key = "Jefe";
    const auto ipad = crypto::hmac::IPadKey::from_key(util::ConstBinaryView(key.c_str(), key.length()), crypto::openssl::md5);
    const auto opad = crypto::hmac::OPadKey::from_key(util::ConstBinaryView(key.c_str(), key.length()), crypto::openssl::md5);
    ASSERT_TRUE(ipad.is_ok());
    ASSERT_TRUE(opad.is_ok());
    const std::string data = "what do ya want for nothing?";
    const auto digest = crypto::hmac::digest({util::ConstBinaryView(data.c_str(), data.length())},
                                             opad.unwrap(),
                                             ipad.unwrap(),
                                             crypto::MD5Hash::Func{&crypto::openssl::md5});
    ASSERT_TRUE(digest.is_ok());
    crypto::MD5Hash::Value expected_v = {0x75, 0x0c, 0x78, 0x3e, 0x6a, 0xb0, 0xb5, 0x03, 0xea, 0xa8, 0x6e, 0x31, 0x0a, 0x5d, 0xb7, 0x38};
    EXPECT_TRUE(crypto::MD5Hash(std::move(expected_v)) == digest.unwrap().value);
}

TEST_F(CryptoHMACOpenSSLTests, rfc2104_test_vectors_3) {
    // key =         0xAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    // key_len       16 bytes
    // data =        0xDDDDDDDDDDDDDDDDDDDD...
    //               ..DDDDDDDDDDDDDDDDDDDD...
    //               ..DDDDDDDDDDDDDDDDDDDD...
    //               ..DDDDDDDDDDDDDDDDDDDD...
    //               ..DDDDDDDDDDDDDDDDDDDD
    // data_len =    50 bytes
    // digest =      0x56be34521d144c88dbb8c733f0e8b3f6
    const auto key = std::vector<uint8_t>(16, 0xAA);
    const auto ipad = crypto::hmac::IPadKey::from_key(util::ConstBinaryView(key), crypto::openssl::md5);
    const auto opad = crypto::hmac::OPadKey::from_key(util::ConstBinaryView(key), crypto::openssl::md5);
    ASSERT_TRUE(ipad.is_ok());
    ASSERT_TRUE(opad.is_ok());
    const auto data = std::vector<uint8_t>(50, 0xDD);
    const auto digest = crypto::hmac::digest({util::ConstBinaryView(data)},
                                             opad.unwrap(),
                                             ipad.unwrap(),
                                             crypto::MD5Hash::Func{&crypto::openssl::md5});
    ASSERT_TRUE(digest.is_ok());
    crypto::MD5Hash::Value expected_v = {0x56, 0xbe, 0x34, 0x52, 0x1d, 0x14, 0x4c, 0x88, 0xdb, 0xb8, 0xc7, 0x33, 0xf0, 0xe8, 0xb3, 0xf6};
    EXPECT_TRUE(crypto::MD5Hash(std::move(expected_v)) == digest.unwrap().value);
}

}
