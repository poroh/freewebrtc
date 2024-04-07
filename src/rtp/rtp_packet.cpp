//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP Packet routines
//

#include <cassert>
#include "rtp/rtp_packet.hpp"
#include "rtp/rtp_error.hpp"
#include "rtp/rtp_payload_map.hpp"
#include "rtp/details/rtp_header_details.hpp"

namespace freewebrtc::rtp {

Result<Packet> Packet::parse(const util::ConstBinaryView& vv, const PayloadMap& ptmap, ParseStat& stat) noexcept {
    using namespace details;

    if (vv.size() < RTP_FIXED_HEADER_LEN) {
        stat.invalid_size.inc();
        stat.error.inc();
        return make_error_code(Error::packet_is_too_short);
    }

    const auto first_byte      = vv.assured_read_u8(0);
    const auto second_byte     = vv.assured_read_u8(1);
    const auto sequence_value  = vv.assured_read_u16be(RTP_SEQUENCE_NUMBER_OFFSET);
    const auto timestamp_value = vv.assured_read_u32be(RTP_TIMESTAMP_OFFSET);
    const auto ssrc_value      = vv.assured_read_u32be(RTP_SSRC_OFFSET);

    if ((first_byte & RTP_VERSION_MASK) != (RTP_VERSION << RTP_VERSION_SHIFT)) {
        stat.invalid_version.inc();
        stat.error.inc();
        return make_error_code(Error::unknown_packet_version);
    }
    const bool has_padding   = (first_byte & RTP_PADDING_MASK) != 0;
    const bool has_extension = (first_byte & RTP_EXTENSION_MASK) != 0;
    const unsigned num_cc = (first_byte & RTP_CC_MASK);

    std::vector<SSRC> csrcs;
    csrcs.reserve(num_cc);
    for (unsigned i = 0; i < num_cc; ++i) {
        const auto maybe_err = vv.read_u32be(RTP_FIXED_HEADER_LEN + i * sizeof(uint32_t))
            .require()
            .bind_err([&](auto&& err) {
                stat.invalid_csrc.inc();
                stat.error.inc();
                return err;
            })
            .fmap([&](auto&& csrc) {
                csrcs.emplace_back(SSRC::from_uint32(csrc));
                return Unit{};
            });
        if (maybe_err.is_err()) {
            return maybe_err.unwrap_err();
        }
    }

    const MarkerBit marker(second_byte & RTP_MARKER_MASK);
    const auto maybe_payload_type = PayloadType::from_uint8(second_byte & RTP_PAYLOAD_TYPE_MASK);
    if (!maybe_payload_type.is_some()) {
        stat.invalid_payload_type.inc();
        stat.error.inc();
        return make_error_code(Error::invalid_payload_type);
    }
    const auto payload_type = maybe_payload_type.unwrap();
    const auto maybe_rtp_clock = ptmap.rtp_clock_rate(payload_type);
    if (!maybe_rtp_clock.is_some()) {
        stat.unknown_rtp_clock.inc();
        stat.error.inc();
        return make_error_code(Error::unknown_rtp_clock);
    }

    const auto rtp_clock = maybe_rtp_clock.unwrap();
    const auto ext_offset = num_cc * sizeof(uint32_t) + RTP_FIXED_HEADER_LEN;

    using MaybeExtension = Maybe<Header::Extension>;
    MaybeExtension maybe_extension = none();
    size_t ext_size = 0;

    if (has_extension) {
        //  0                   1                   2                   3
        //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |      defined by profile       |           length              |
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |                        header extension                       |
        // |                             ....                              |
        //
        // The header extension contains a 16-bit length field that counts
        // the number of 32-bit words in the extension, excluding the
        // four-octet extension header (therefore zero is a valid length)
        const auto maybe_err = vv.read_u16be(ext_offset + 2)
            .require()
            .bind_err([&](auto&&) {
                stat.invalid_extension.inc();
                stat.error.inc();
                return make_error_code(Error::invalid_extension_length);
            })
            .bind([&](auto&& ext_length) -> MaybeError {
                ext_size = (ext_length + 1) * sizeof(uint32_t);
                const util::ConstBinaryView::Interval ext_interval{ext_offset + sizeof(uint32_t), ext_size - sizeof(uint32_t)};
                if (!vv.contains(ext_interval)) {
                    stat.invalid_extension.inc();
                    stat.error.inc();
                    return make_error_code(Error::invalid_extension_length);
                }
                return vv.read_u16be(ext_offset)
                    .require()
                    .fmap([&](auto&& len) {
                        maybe_extension = Header::Extension{len, ext_interval};
                        return Unit{};
                    });
            });
        if (maybe_err.is_err()) {
            return maybe_err.unwrap_err();
        }
    }

    const auto payload_offset = ext_offset + ext_size;
    assert(vv.size() >= payload_offset); // guaranteed by extension checking or reading fixed header / csrcs
    const size_t payload_with_padding_size = vv.size() - payload_offset;

    size_t padding_size = 0;
    if (has_padding) {
        // If the padding bit is set, the packet contains one or more
        // additional padding octets at the end which are not part of
        // the payload.  The last octet of the padding contains a
        // count of how many padding octets should be ignored,
        // including itself.
        if (payload_with_padding_size == 0) {
            stat.invalid_padding.inc();
            stat.error.inc();
            return make_error_code(Error::invalid_packet_padding);
        }
        auto maybe_err = vv.read_u8(vv.size() - 1)
            .require()
            .bind_err([&](auto&&) {
                stat.invalid_padding.inc();
                stat.error.inc();
                return make_error_code(Error::invalid_packet_padding);
            })
            .bind([&](auto&& size) -> MaybeError {
                padding_size = size;
                if (padding_size > payload_with_padding_size) {
                    stat.invalid_padding.inc();
                    stat.error.inc();
                    return make_error_code(Error::invalid_packet_padding);
                }
                return Unit{};
            });
        if (maybe_err.is_err()) {
            return maybe_err.unwrap_err();
        }
    }

    const size_t payload_size = payload_with_padding_size - padding_size;
    stat.success.inc();
    return Packet {
        Header {marker,
                payload_type,
                SequenceNumber::from_uint16(sequence_value),
                SSRC::from_uint32(ssrc_value),
                Timestamp::from_uint32(timestamp_value, rtp_clock),
                std::move(csrcs),
                std::move(maybe_extension)},
        util::ConstBinaryView::Interval{payload_offset, payload_size}
    };
}

}
