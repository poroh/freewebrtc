//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Interactive Connectivity Establishment (ICE)
// ICE Candidate in SDP
// RFC8839
//

#include "ice/candidate/ice_candidate_sdp.hpp"
#include "ice/candidate/ice_candidate_error.hpp"
#include "util/util_string_view.hpp"
#include "util/util_token_stream.hpp"

namespace freewebrtc::ice::candidate {

// candidate-attribute   = "candidate" ":" foundation SP component-id SP
//                         transport SP
//                         priority SP
//                         connection-address SP     ;from RFC 4566
//                         port         ;port from RFC 4566
//                         SP cand-type
//                         [SP rel-addr]
//                         [SP rel-port]
//                         *(SP cand-extension)
//
// foundation            = 1*32ice-char
// component-id          = 1*3DIGIT
// transport             = "UDP" / transport-extension
// transport-extension   = token              ; from RFC 3261
// priority              = 1*10DIGIT
// cand-type             = "typ" SP candidate-types
// candidate-types       = "host" / "srflx" / "prflx" / "relay" / token
// rel-addr              = "raddr" SP connection-address
// rel-port              = "rport" SP port
// cand-extension        = extension-att-name SP extension-att-value
// extension-att-name    = token
// extension-att-value   = *VCHAR
// ice-char              = ALPHA / DIGIT / "+" / "/"
ReturnValue<SDPAttrParseResult> parse_sdp_attr(std::string_view inv) {
    using SV = std::string_view;
    using MaybeSV = std::optional<std::string_view>;
    auto advance = [](const MaybeSV& v, size_t sz) -> MaybeSV {
        if (!v.has_value()) {
            return std::nullopt;
        }
        if (sz > v->size()) {
            return std::nullopt;
        }
        return SV{v->data() + sz, v->size() - sz};
    };
    static constexpr SV prefix("candidate:");
    if (!inv.starts_with(prefix)) {
        return make_error_code(Error::invalid_attr_prefix);
    }
    auto maybev = advance(inv, prefix.size());
    if (!maybev.has_value()) {
        return make_error_code(Error::invalid_attr_prefix);
    }
    auto& v = maybev.value();
    auto tstr = util::TokenStream{util::string_view::split_all(v, ' ')};

    auto foundation_rv      = tstr.required_bind(Foundation::from_string).add_context("foundation");
    auto component_id_rv    = tstr.required_bind(ComponentId::from_string).add_context("component");
    auto transport_rv       = tstr.required_bind(TransportType::from_string).add_context("transport");
    auto priority_rv        = tstr.required_bind(Priority::from_string).add_context("priority");
    auto connection_addr_rv = tstr.required_bind(Address::from_string).add_context("address");
    auto port_rv            = tstr.required_bind(net::Port::from_string).add_context("port");
    auto type_rv = tstr.required("typ")
        .bind([&](auto&&) { return tstr.required_bind(Type::from_string); });
    return combine([](auto&& f, auto&& cid, auto& t, auto&& p, auto&& addr, auto&& port, auto&& type) -> ReturnValue<SDPAttrParseResult> {
        return SDPAttrParseResult{ Supported {
            .candidate = {
                .address = std::move(addr),
                .port = std::move(port),
                .transport_type = std::move(t),
                .foundation = std::move(f),
                .maybe_component = std::move(cid),
                .priority = std::move(p),
                .type = std::move(type)
            }
        } };
    }
    , std::move(foundation_rv)
    , std::move(component_id_rv)
    , std::move(transport_rv)
    , std::move(priority_rv)
    , std::move(connection_addr_rv)
    , std::move(port_rv)
    , std::move(type_rv));
}


}
