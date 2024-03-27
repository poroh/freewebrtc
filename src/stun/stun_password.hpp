//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Password
//
// STUN defines many ways of wrapping password
// This module defines this algorithms
// https://www.rfc-editor.org/rfc/rfc8489.html#section-18.5.1
//

#pragma once

#include <cstdint>
#include <vector>

#include "util/util_result.hpp"
#include "crypto/crypto_hash.hpp"
#include "crypto/crypto_hmac.hpp"
#include "precis/precis_opaque_string.hpp"

namespace freewebrtc::stun {

class Password {
public:
    Password(const Password&) = default;
    Password(Password&&) = default;
    Password& operator=(const Password&) = default;
    bool operator==(const Password&) const noexcept = default;

    const crypto::hmac::IPadKey& ipad() const noexcept;
    const crypto::hmac::OPadKey& opad() const noexcept;

    using OpaqueString = precis::OpaqueString;

    static Result<Password> short_term(const OpaqueString& password, crypto::SHA1Hash::Func);
    // TODO: Not implemented yet:
    // static Password long_term_md5(const OpaqueString& username, const OpaqueString& realm, const OpaqueString& password, crypto::MD5Hash::Func);
    // static Password long_term_sha256(const OpaqueString& username, const OpaqueString& realm, const OpaqueString& password, crypto::SHA256Hash::Func);

private:
    Password(crypto::hmac::IPadKey&&, crypto::hmac::OPadKey&&);
    crypto::hmac::IPadKey m_ipad;
    crypto::hmac::OPadKey m_opad;
};


//
// inlines
//
inline const crypto::hmac::IPadKey& Password::ipad() const noexcept {
    return m_ipad;
}

inline const crypto::hmac::OPadKey& Password::opad() const noexcept {
    return m_opad;
}

}
