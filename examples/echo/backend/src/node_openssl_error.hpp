//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// OpenSSL std::error_code wrapper
//

#include <system_error>

#pragma once

namespace freewebrtc::crypto::node_openssl {

class OpenSSLErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override;
    std::string message(int code) const override;
};

std::error_code make_error_code(int code);

}


