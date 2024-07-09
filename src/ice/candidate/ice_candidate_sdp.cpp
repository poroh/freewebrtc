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
#include "util/util_unit.hpp"

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
    using MaybeSV = Maybe<std::string_view>;
    auto advance = [](const MaybeSV& v, size_t sz) -> MaybeSV {
        return v.bind([&](auto&& v) -> MaybeSV {
            if (sz > v.size()) {
                return none();
            }
            return SV{v.data() + sz, v.size() - sz};
        });
    };
    static constexpr SV prefix("candidate:");
    if (!inv.starts_with(prefix)) {
        return make_error_code(Error::invalid_attr_prefix);
    }
    auto maybev = advance(inv, prefix.size());
    if (!maybev.is_some()) {
        return make_error_code(Error::invalid_attr_prefix);
    }
    auto& v = maybev.unwrap();
    auto tstr = util::TokenStream{util::string_view::split_all(v, ' ')};

    auto foundation_rv      = tstr.required_bind(Foundation::from_string).add_context("foundation");
    auto component_id_rv    = tstr.required_bind(ComponentId::from_string).add_context("component");
    auto transport_rv       = tstr.required_bind(TransportType::from_string).add_context("transport");
    auto priority_rv        = tstr.required_bind(Priority::from_string).add_context("priority");
    auto connection_addr_rv = tstr.required_bind(Address::from_string).add_context("address");
    auto port_rv            = tstr.required_bind(net::Port::from_string).add_context("port");
    auto type_rv = tstr.required("typ")
        .bind([&](auto&&) { return tstr.required_bind(Type::from_string); });

    Maybe<Result<Address>> maybe_raddr = none();
    Maybe<Result<net::Port>> maybe_rport = none();
    Supported::ExtensionVec extensions;
    while (true) {
        auto maybe_att_name = tstr.optional();
        if (maybe_att_name.is_none()) {
            break;
        }
        auto att_name = maybe_att_name.unwrap();
        if (att_name == "raddr") {
            maybe_raddr = tstr.required_bind(Address::from_string).add_context("raddr");
        } else if (att_name == "rport") {
            maybe_rport = tstr.required_bind(net::Port::from_string).add_context("rport");
        } else {
            auto maybe_err =
                tstr.required()
                .fmap([&](auto&& att_value) {
                    extensions.emplace_back(Supported::Extension{
                        .att_name = std::string{std::move(att_name)},
                        .att_value = std::string{std::move(att_value)}
                    });
                    return Unit::create();
                });
            if (maybe_err.is_err()) {
                return maybe_err.unwrap_err();
            }
        }
    }

    // RFC8839:
    // An agent processing remote candidates MUST ignore "candidate"
    // lines that include candidates with FQDNs or IP address versions
    // that are not supported or recognized.
    const auto unsupported = any_is_err(connection_addr_rv, type_rv, transport_rv);
    if (unsupported.is_err()) {
        auto error = unsupported.unwrap_err();
        if (connection_addr_rv.is_err()
            || error == make_error_code(Error::unknown_candidate_type)
            || error == make_error_code(Error::unknown_transport_type)) {
            return SDPAttrParseResult{ Unsupported{ error.message() } };
        }
    }

    using MaybeAddr = Maybe<Address>;
    using MaybeAddrRV = Result<MaybeAddr>;
    MaybeAddrRV maybe_raddr_rv =  std::move(maybe_raddr)
        .fmap([](Result<Address>&& addr_rv) {
            return std::move(addr_rv).fmap(MaybeAddr::move_from);
        })
        .value_or(MaybeAddr::none());

    using MaybePort = Maybe<net::Port>;
    using MaybePortRV = Result<MaybePort>;
    MaybePortRV maybe_rport_rv =  maybe_rport
        .fmap([](const Result<net::Port>& rport_rv) {
            return rport_rv.fmap(MaybePort::copy_from);
        })
        .value_or(MaybePort::none());

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
                        .component = std::move(cid),
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
