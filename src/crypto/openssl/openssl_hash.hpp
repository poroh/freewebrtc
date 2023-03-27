//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// OpenSSL implementation for hash functions
//

#pragma once

#include "crypto/crypto_hash.hpp"

namespace freewebrtc::crypto::openssl {

crypto::SHA1Hash::Result sha1(const crypto::SHA1Hash::Input&);
crypto::MD5Hash::Result md5(const crypto::MD5Hash::Input&);

}




