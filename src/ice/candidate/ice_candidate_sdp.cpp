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
    auto tokens = util::string_view::split_all(v, ' ');
    if (tokens.size() < 6) {
        return make_error_code(Error::invalid_candidate_parts_number);
    }
    return make_error_code(Error::invalid_attr_prefix);
}


}
