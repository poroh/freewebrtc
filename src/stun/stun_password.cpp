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
    return combine([](crypto::hmac::IPadKey&& ipad, crypto::hmac::OPadKey&& opad) -> ReturnValue<Password> {
        return Password(std::move(ipad), std::move(opad));
    }, std::move(ipad), std::move(opad));
}

Password::Password(crypto::hmac::IPadKey&& ipad, crypto::hmac::OPadKey&& opad)
    : m_ipad(std::move(ipad))
    , m_opad(std::move(opad))
{}

}

