//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Fully Qualified Domain Name
//

#pragma once

#include "util/util_parse_result.hpp"
#include "util/util_return_value.hpp"

namespace freewebrtc::net {

class Fqdn {
public:
    Fqdn(const Fqdn&) = default;
    Fqdn(Fqdn&&) = default;
    Fqdn& operator=(const Fqdn&) = default;
    Fqdn& operator=(Fqdn&&) = default;

    static ParseResult<Fqdn> parse(std::string_view);
    static ReturnValue<Fqdn> from_string(std::string_view);

    const std::string& to_string() const noexcept;

private:
    explicit Fqdn(std::string_view v);
    std::string m_value;
};


//
// inlines
//
inline const std::string& Fqdn::to_string() const noexcept {
    return m_value;
}

}
