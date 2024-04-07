//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Error in freewebrtc library
//
// Motivation to implement this class is that
// std::error_code cannot provide context to error
// So it may be hard to narrow down in what context
// Error occur.
//

#pragma once

#include <system_error>
#include <vector>
#include <string>

namespace freewebrtc {

class Error {
public:
    Error(std::error_code&&);
    Error(const std::error_code&);
    Error(const Error&) = default;
    Error(Error&&) = default;
    Error& operator=(const Error&) = default;
    Error& operator=(Error&&) = default;

    Error& add_context(const std::string&);
    Error& add_context(std::string&&);
    template<typename... Ts>
    Error& add_context(Ts&&...);

    bool operator==(const std::error_code&) const noexcept;

    std::string message() const;
    const std::error_category& category() const noexcept;
    int value() const noexcept;

private:
    using ContextVec = std::vector<std::string>;
    template<typename T, typename... Ts>
    void add_context_impl(ContextVec& v, T&& t, Ts&&... rest);
    std::error_code m_code;
    ContextVec m_context;
};

//
// inlines
//
inline Error::Error(std::error_code&& c)
    : m_code(std::move(c))
{}

inline Error::Error(const std::error_code& c)
    : m_code(c)
{}

inline Error& Error::add_context(const std::string& s) {
    m_context.emplace_back(s);
    return *this;
}

inline Error& Error::add_context(std::string&& s) {
    m_context.emplace_back(std::move(s));
    return *this;
}

template<typename... Ts>
Error& Error::add_context(Ts&&... ts) {
    ContextVec v;
    add_context_impl(v, std::forward<Ts>(ts)...);
    m_context.reserve(m_context.size() + v.size());
    for (auto rit = v.rbegin(); rit != v.rend(); ++rit) {
        // add in reverse order
        m_context.emplace_back(std::move(*rit));
    }
    return *this;
}


inline bool Error::operator==(const std::error_code& c) const noexcept {
    return m_code == c;
}

inline const std::error_category& Error::category() const noexcept {
    return m_code.category();
}

inline int Error::value() const noexcept {
    return m_code.value();
}

template<typename T, typename... Ts>
inline void Error::add_context_impl(ContextVec& v, T&& t, Ts&&... rest) {
    if constexpr (sizeof...(rest) == 0) {
        v.emplace_back(std::forward<T>(t));
    } else {
        v.emplace_back(std::forward<T>(t));
        add_context_impl(v, std::forward<Ts>(rest)...);
    }
}


}
