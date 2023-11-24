//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// OpenSSL std::error_code wrapper
//

#include "openssl/err.h"
#include "node_openssl_error.hpp"

namespace freewebrtc::crypto::node_openssl {

namespace {

const OpenSSLErrorCategory& openssl_category() {
    static const OpenSSLErrorCategory cat;
    return cat;
}

}

const char *OpenSSLErrorCategory::name() const noexcept {
    return "OpenSSL";
}

std::string OpenSSLErrorCategory::message(int code) const {
    char buf[256];
    ERR_error_string_n(code, buf, sizeof(buf));
    return buf;
}

std::error_code make_error_code(int code) {
    return std::error_code(code, openssl_category());
}

}


