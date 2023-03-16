
#include "rtp/RTPPacket.hpp"
#include "rtp/RTPPayloadMap.hpp"

namespace freewebrtc::rtp {

std::optional<Packet> Packet::parse(const util::ConstBinaryView& vv, const PayloadMap& ptmap, ParseStat& stat) noexcept {

    //  0                   1                   2                   3
    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |V=2|P|X|  CC   |M|     PT      |       sequence number         |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                           timestamp                           |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |           synchronization source (SSRC) identifier            |
    // +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
    // |            contributing source (CSRC) identifiers             |
    // |                             ....                              |
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    static constexpr size_t  RTP_FIXED_HEADER_LEN = 12;
    static constexpr uint8_t RTP_VERSION = 2;
    static constexpr uint8_t RTP_VERSION_MASK = 0xC0;
    static constexpr int     RTP_VERSION_SHIFT = 6;
    static constexpr uint8_t RTP_PADDING_MASK = 0x20;
    static constexpr uint8_t RTP_EXTENSION_MASK = 0x10;
    static constexpr uint8_t RTP_CC_MASK = 0x0F;
    static constexpr uint8_t RTP_MARKER_MASK = 0x80;
    static constexpr uint8_t RTP_PAYLOAD_TYPE_MASK = 0x7F;
    static constexpr size_t  RTP_SEQUENCE_NUMBER_OFFSET = 2;
    static constexpr size_t  RTP_TIMESTAMP_OFFSET = 4;
    static constexpr size_t  RTP_SSRC_OFFSET = 8;

    if (vv.size() < RTP_FIXED_HEADER_LEN) {
        stat.truncated.inc();
        stat.error.inc();
        return std::nullopt;
    }

    const auto first_byte      = vv.assured_read_u8(0);
    const auto second_byte     = vv.assured_read_u8(1);
    const auto sequence_value  = vv.assured_read_u16be(RTP_SEQUENCE_NUMBER_OFFSET);
    const auto timestamp_value = vv.assured_read_u32be(RTP_TIMESTAMP_OFFSET);
    const auto ssrc_value      = vv.assured_read_u32be(RTP_SSRC_OFFSET);

    if ((first_byte & RTP_VERSION_MASK) != (RTP_VERSION << RTP_VERSION_SHIFT)) {
        stat.invalid_version.inc();
        stat.error.inc();
        return std::nullopt;
    }
    const bool has_padding   = (first_byte & RTP_PADDING_MASK) != 0;
    const bool has_extension = (first_byte & RTP_EXTENSION_MASK) != 0;
    const unsigned num_cc = (first_byte & RTP_CC_MASK);

    std::vector<SSRC> cssrc;
    cssrc.reserve(num_cc);
    for (unsigned i = 0; i < num_cc; ++i) {
        const auto maybe_csrc = vv.read_u32be(RTP_FIXED_HEADER_LEN + i * sizeof(uint32_t));
        if (!maybe_csrc.has_value()) {
            stat.invalid_csrc.inc();
            stat.error.inc();
            return std::nullopt;
        }
        cssrc.emplace_back(SSRC::from_uint32(*maybe_csrc));
    }

    const MarkerBit marker(second_byte & RTP_MARKER_MASK);
    const auto maybe_payload_type = PayloadType::from_uint8(second_byte & RTP_PAYLOAD_TYPE_MASK);
    if (!maybe_payload_type.has_value()) {
        stat.invalid_payload_type.inc();
        stat.error.inc();
        return std::nullopt;
    }
    const auto payload_type = *maybe_payload_type;
    const auto maybe_rtp_clock = ptmap.rtp_clock_rate(payload_type);
    if (!maybe_rtp_clock.has_value()) {
        stat.unknown_rtp_clock.inc();
        stat.error.inc();
        return std::nullopt;
    }

    const auto rtp_clock = *maybe_rtp_clock;
    const auto ext_offset = num_cc * sizeof(uint32_t) + RTP_FIXED_HEADER_LEN;

    using MaybeExtension = std::optional<Header::Extension>;
    MaybeExtension maybe_extension;
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
        const auto maybe_ext_length = vv.read_u16be(ext_offset + 2);
        if (!maybe_ext_length.has_value()) {
            stat.invalid_extension.inc();
            stat.error.inc();
            return std::nullopt;
        }
        ext_size = (*maybe_ext_length + 1) * sizeof(uint32_t);
        const util::ConstBinaryView::Interval ext_interval{ext_offset + sizeof(uint32_t), ext_size - sizeof(uint32_t)};
        if (!vv.contains(ext_interval)) {
            stat.invalid_extension.inc();
            stat.error.inc();
            return std::nullopt;
        }
        maybe_extension = MaybeExtension{Header::Extension{*vv.read_u16be(ext_offset), ext_interval}};
    }

    const auto payload_offset = ext_offset + ext_size;
    assert(vv.size() >= payload_offset); // guaranteed by extension checking or reading fixed header / cssrc
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
            return std::nullopt;
        }
        padding_size = *vv.read_u8(vv.size() - 1);
    }

    if (padding_size > payload_with_padding_size) {
        stat.invalid_padding.inc();
        stat.error.inc();
        return std::nullopt;
    }
    const size_t payload_size = payload_with_padding_size - padding_size;
    stat.success.inc();
    return Packet {
        Header {marker,
                payload_type,
                SequenceNumber::from_uint16(sequence_value),
                SSRC::from_uint32(ssrc_value),
                Timestamp::from_uint32(timestamp_value, rtp_clock),
                std::move(cssrc),
                maybe_extension},
        util::ConstBinaryView::Interval{payload_offset, payload_size}
    };
}

}
