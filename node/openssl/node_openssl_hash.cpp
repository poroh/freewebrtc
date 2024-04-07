//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// OpenSSL implementation for hash functions
//

#include <memory>

#include "openssl/evp.h"
#include "openssl/err.h"

#include "node_openssl_hash.hpp"
#include "node_openssl_error.hpp"

namespace freewebrtc::crypto::node_openssl {

class MessageDigest {
public:
    explicit MessageDigest(const EVP_MD *md);

    template<typename Hash>
    typename Hash::Result calc(const typename Hash::Input& input);

private:
    MaybeError init();
    MaybeError update(const util::ConstBinaryView& view);
    MaybeError finalize(uint8_t *result);

    using EVPMDContextPtr = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;
    EVPMDContextPtr m_ctx;
    const EVP_MD * const m_md;
};


crypto::SHA1Hash::Result sha1(const crypto::SHA1Hash::Input& input) {
    return MessageDigest(EVP_sha1()).calc<crypto::SHA1Hash>(input);
}

crypto::MD5Hash::Result md5(const crypto::MD5Hash::Input& input) {
    return MessageDigest(EVP_md5()).calc<crypto::MD5Hash>(input);
}

// MessageDigest implementation
MessageDigest::MessageDigest(const EVP_MD *md)
    : m_ctx(EVP_MD_CTX_new(), &EVP_MD_CTX_free)
    , m_md(md)
{}

template<typename Hash>
typename Hash::Result MessageDigest::calc(const typename Hash::Input& input) {
    if (auto maybe_err = init(); maybe_err.is_err()) {
        return maybe_err.unwrap_err();
    }

    for (const auto& chunk: input) {
        update(chunk);
    }

    typename Hash::Value v;
    if (auto maybe_err = finalize(v.data()); maybe_err.is_err()) {
        return maybe_err.unwrap_err();
    }
    return Hash{std::move(v)};
}

MaybeError MessageDigest::init() {
    if (m_ctx == nullptr) {
        return std::make_error_code(std::errc::not_enough_memory);
    }
    ERR_clear_error();
    if (EVP_DigestInit_ex(m_ctx.get(), m_md, NULL) != 1) {
        return make_error_code(ERR_get_error());
    }
    return success();
}

MaybeError MessageDigest::update(const util::ConstBinaryView& view) {
    ERR_clear_error();
    if (EVP_DigestUpdate(m_ctx.get(), view.data(), view.size()) != 1) {
        return make_error_code(ERR_get_error());
    }
    return success();
}

MaybeError MessageDigest::finalize(uint8_t *result) {
    ERR_clear_error();
    if (EVP_DigestFinal_ex(m_ctx.get(), result, NULL) != 1) {
        return make_error_code(ERR_get_error());
    }
    return success();
}

}



