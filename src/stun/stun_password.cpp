//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Password
//

#include "stun/stun_password.hpp"

namespace freewebrtc::stun {

ReturnValue<Password> Password::short_term(const OpaqueString& password, crypto::SHA1Hash::Func h) {
    std::vector<uint8_t> data(password.value.begin(), password.value.end());
    auto ipad = crypto::hmac::IPadKey::from_key(util::ConstBinaryView(data), h);
    auto opad = crypto::hmac::OPadKey::from_key(util::ConstBinaryView(data), h);
    if (ipad.is_error() || opad.is_error()) {
        return ipad.maybe_error().value_or(opad.assert_error());
    }
    return Password(std::move(ipad.assert_value()), std::move(opad.assert_value()));
}

Password::Password(crypto::hmac::IPadKey&& ipad, crypto::hmac::OPadKey&& opad)
    : m_ipad(std::move(ipad))
    , m_opad(std::move(opad))
{}

}

