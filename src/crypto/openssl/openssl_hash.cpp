//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// OpenSSL implementation for hash functions
//

#include <openssl/evp.h>
#include <openssl/err.h>

#include "crypto/openssl/openssl_hash.hpp"
#include "crypto/openssl/openssl_error.hpp"

namespace freewebrtc::crypto::openssl {


class MessageDigest {
public:
    explicit MessageDigest(const EVP_MD *md)
        : m_ctx(EVP_MD_CTX_new(), &EVP_MD_CTX_free)
        , m_md(md)
    {}
    std::optional<std::error_code> init() {
        if (m_ctx == nullptr) {
            return std::make_error_code(std::errc::not_enough_memory);
        }
        ERR_clear_error();
        if (EVP_DigestInit_ex(m_ctx.get(), m_md, NULL) != 1) {
            return make_error_code(ERR_get_error());
        }
        return std::nullopt;
    }
    std::optional<std::error_code> update(const util::ConstBinaryView& view) {
        ERR_clear_error();
        if (EVP_DigestUpdate(m_ctx.get(), view.data(), view.size()) != 1) {
            return make_error_code(ERR_get_error());
        }
        return std::nullopt;
    }
    std::optional<std::error_code> finalize(uint8_t *result) {
        ERR_clear_error();
        if (EVP_DigestFinal_ex(m_ctx.get(), result, NULL) != 1) {
            return make_error_code(ERR_get_error());
        }
        return std::nullopt;
    }
private:
    using EVPMDContextPtr = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;
    EVPMDContextPtr m_ctx;
    const EVP_MD * const m_md;
};

crypto::SHA1Hash::Result sha1(const crypto::SHA1Hash::Input& input) {
    MessageDigest md(EVP_sha1());

    if (auto maybe_err = md.init(); maybe_err.has_value()) {
        return *maybe_err;
    }

    for (const auto& chunk: input) {
        md.update(chunk);
    }

    SHA1Hash::Value v;
    if (auto maybe_err = md.finalize(v.data()); maybe_err.has_value()) {
        return *maybe_err;
    }
    return crypto::SHA1Hash{std::move(v)};
}

crypto::MD5Hash::Result md5(const crypto::MD5Hash::Input& input) {
    MessageDigest md(EVP_md5());

    if (auto maybe_err = md.init(); maybe_err.has_value()) {
        return *maybe_err;
    }

    for (const auto& chunk: input) {
        md.update(chunk);
    }

    MD5Hash::Value v;
    if (auto maybe_err = md.finalize(v.data()); maybe_err.has_value()) {
        return *maybe_err;
    }
    return crypto::MD5Hash{std::move(v)};
}

}



