//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// STUN Header
//

#include "stun/stun_header.hpp"
#include "stun/details/stun_constants.hpp"
#include "util/util_endian.hpp"

namespace freewebrtc::stun {

util::ByteVec Header::build(size_t len) const {
    const uint16_t msg_type = util::host_to_network_u16(cls.to_msg_type() | method.to_msg_type());
    const uint16_t msg_size = util::host_to_network_u16(len);
    if (transaction_id.view().size() == details::TRANSACTION_ID_SIZE) {
        // With magic cookie
        const uint32_t magic_cookie = util::host_to_network_u32(details::MAGIC_COOKIE);
        return util::ConstBinaryView::concat({
                util::ConstBinaryView(&msg_type, sizeof(msg_type)),
                util::ConstBinaryView(&msg_size, sizeof(msg_size)),
                util::ConstBinaryView(&magic_cookie, sizeof(magic_cookie)),
                transaction_id.view()
            });
    } else {
        // No magic cookie (backward compatibility to RFC3489)
        return util::ConstBinaryView::concat({
                util::ConstBinaryView(&msg_type, sizeof(msg_type)),
                util::ConstBinaryView(&msg_size, sizeof(msg_size)),
                transaction_id.view()
            });
    }
}

}
