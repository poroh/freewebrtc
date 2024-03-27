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
Result<SDPAttrParseResult> parse_sdp_attr(std::string_view inv) {
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

    std::optional<Result<Address>> maybe_raddr;
    std::optional<Result<net::Port>> maybe_rport;
    Supported::ExtensionVec extensions;
    while (true) {
        auto maybe_att_name = tstr.optional();
        if (!maybe_att_name.has_value()) {
            break;
        }
        auto att_name = maybe_att_name.value();
        if (att_name == "raddr") {
            maybe_raddr = tstr.required_bind(Address::from_string).add_context("raddr");
        } else if (att_name == "rport") {
            maybe_rport = tstr.required_bind(net::Port::from_string).add_context("rport");
        } else {
            auto att_value_rv = tstr.required();
            if (att_value_rv.is_ok()) {
                extensions.emplace_back(Supported::Extension{
                        .att_name = std::string{att_name},
                        .att_value = std::string{att_value_rv.unwrap()}
                    });
            }
        }
    }

    // RFC8839:
    // An agent processing remote candidates MUST ignore "candidate"
    // lines that include candidates with FQDNs or IP address versions
    // that are not supported or recognized.
    const auto unsupported = any_is_err(connection_addr_rv, type_rv, transport_rv);
    if (unsupported.has_value()) {
        auto error = unsupported.value();
        if (connection_addr_rv.is_err()
            || error == make_error_code(Error::unknown_candidate_type)
            || error == make_error_code(Error::unknown_transport_type)) {
            return SDPAttrParseResult{ Unsupported{ error.message() } };
        }
    }

    using MaybeAddr = std::optional<Address>;
    using MaybeAddrRV = Result<MaybeAddr>;
    MaybeAddrRV maybe_raddr_rv =  maybe_raddr.has_value()
        ? std::move(maybe_raddr.value()).fmap([](auto&& addr) { return MaybeAddr{std::move(addr)}; })
        : MaybeAddrRV{std::nullopt};

    using MaybePort = std::optional<net::Port>;
    using MaybePortRV = Result<MaybePort>;
    MaybePortRV maybe_rport_rv =  maybe_rport.has_value()
        ? maybe_rport.value().fmap([](auto&& port) { return MaybePort{port}; })
        : MaybePortRV{std::nullopt};

    return combine([](
            auto&& f, auto&& cid, auto& t, auto&& p,
            auto&& addr, auto&& port, auto&& type,
            auto&& raddr, auto&& rport,
            auto&& extensions
            ) -> Result<SDPAttrParseResult>
        {
            return SDPAttrParseResult{
                Supported {
                    .candidate = {
                        .address = std::move(addr),
                        .port = std::move(port),
                        .transport_type = std::move(t),
                        .foundation = std::move(f),
                        .maybe_component = std::move(cid),
                        .priority = std::move(p),
                        .type = std::move(type),
                        .maybe_related_address = std::move(raddr),
                        .maybe_related_port = std::move(rport)
                    },
                    .extensions = std::move(extensions)
                }
            };
        }
        , std::move(foundation_rv)
        , std::move(component_id_rv)
        , std::move(transport_rv)
        , std::move(priority_rv)
        , std::move(connection_addr_rv)
        , std::move(port_rv)
        , std::move(type_rv)
        , std::move(maybe_raddr_rv)
        , std::move(maybe_rport_rv)
        , return_value(std::move(extensions)));
}

}
