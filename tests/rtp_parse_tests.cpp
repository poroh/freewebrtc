//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// RTP parsing tests
//

#include <gtest/gtest.h>

#include "util/util_flat.hpp"
#include "rtp/rtp_packet.hpp"
#include "rtp/rtp_payload_map.hpp"
#include "helpers/rtp_packet_helpers.hpp"
#include "helpers/endian_helpers.hpp"

namespace freewebrtc::test {

class RTPPacketParserTest : public ::testing::Test {
};

// ================================================================================
// Positive cases

// Empty payload test:
TEST_F(RTPPacketParserTest, empty_packet_test) {
    rtp::ParseStat stat;
    const auto pt = rtp::PayloadType::from_uint8(0).unwrap(); // PCMU
    const auto ssrc = rtp::SSRC::from_uint32(0xDEADBEEF);
    const uint32_t timestamp_value = 160;
    const uint16_t sequence_value = 0x1234;

    rtp::PayloadMap map({std::make_pair(pt, rtp::PayloadMapItem{rtp::ClockRate(8000)})});
    auto result_rv =
        rtp::Packet::parse(
            util::ConstBinaryView(
                util::flat_vec<uint8_t>(
                    {
                        rtp_helpers::first_word(pt.value(), sequence_value),
                        helpers::uint32be(timestamp_value),
                        helpers::uint32be(ssrc.value())
                    }
                )
            ),
            map,
            stat);
    ASSERT_TRUE(result_rv.is_ok());
    auto result = result_rv.unwrap();
    EXPECT_EQ(stat.success.count(), 1);
    EXPECT_TRUE(!result.header.marker);
    EXPECT_EQ(result.header.payload_type, pt);
    EXPECT_EQ(result.header.sequence.value(), sequence_value);
    EXPECT_EQ(result.header.ssrc, ssrc);
    EXPECT_EQ(result.header.timestamp.value(), timestamp_value);
    EXPECT_TRUE(result.header.csrcs.empty());
    EXPECT_FALSE(result.header.maybe_extension.is_some());
    ASSERT_EQ(result.payload.count, 0);
}

// Packet with 4 bytes padding
TEST_F(RTPPacketParserTest, empty_packet_with_padding_test) {
    rtp::ParseStat stat;
    const auto pt = rtp::PayloadType::from_uint8(0).unwrap(); // PCMU
    const auto ssrc = rtp::SSRC::from_uint32(0xDEADBEEF);
    const uint32_t timestamp_value = 160;
    const uint16_t sequence_value = 0x1234;

    rtp::PayloadMap map({std::make_pair(pt, rtp::PayloadMapItem{rtp::ClockRate(8000)})});
    auto result_rv =
        rtp::Packet::parse(
            util::ConstBinaryView(
                util::flat_vec<uint8_t>(
                    {
                        rtp_helpers::first_word(pt.value(), sequence_value, false, true),
                        helpers::uint32be(timestamp_value),
                        helpers::uint32be(ssrc.value()),
                        {0xa, 0xb, 0xc, 0x4}
                    }
                )
            ),
            map,
            stat);
    ASSERT_TRUE(result_rv.is_ok());
    auto result = result_rv.unwrap();
    EXPECT_EQ(stat.success.count(), 1);
    ASSERT_EQ(result.payload.count, 0);
}

// 4-bytes payload test
TEST_F(RTPPacketParserTest, four_bytes_payload_test) {
    rtp::ParseStat stat;
    const auto pt = rtp::PayloadType::from_uint8(0).unwrap(); // PCMU
    rtp::PayloadMap map({std::make_pair(pt, rtp::PayloadMapItem{rtp::ClockRate(8000)})});
    auto result_rv =
        rtp::Packet::parse(
            util::ConstBinaryView(
                util::flat_vec<uint8_t>(
                    {
                        rtp_helpers::first_word(pt.value(), 0x1234),
                        helpers::uint32be(160),
                        helpers::uint32be(0xDEADBEEF),
                        {1, 2, 3, 4}
                    }
                )
            ),
            map,
            stat);
    ASSERT_TRUE(result_rv.is_ok());
    auto result = result_rv.unwrap();
    EXPECT_EQ(stat.success.count(), 1);
    ASSERT_EQ(result.payload.count, 4);
    ASSERT_EQ(result.payload.offset, rtp::details::RTP_FIXED_HEADER_LEN);
}

// 4-bytes payload with 4 bytes padding test
TEST_F(RTPPacketParserTest, four_bytes_payload_with_padding_test) {
    rtp::ParseStat stat;
    const auto pt = rtp::PayloadType::from_uint8(0).unwrap(); // PCMU
    rtp::PayloadMap map({std::make_pair(pt, rtp::PayloadMapItem{rtp::ClockRate(8000)})});
    auto result_rv =
        rtp::Packet::parse(
            util::ConstBinaryView(
                util::flat_vec<uint8_t>(
                    {
                        rtp_helpers::first_word(pt.value(), 0x1234, false, true),
                        helpers::uint32be(160),
                        helpers::uint32be(0xDEADBEEF),
                        {1, 2, 3, 4},
                        {0xa, 0xb, 0xc, 4}
                    }
                )
            ),
            map,
            stat);
    ASSERT_TRUE(result_rv.is_ok());
    auto result = result_rv.unwrap();
    EXPECT_EQ(stat.success.count(), 1);
    ASSERT_EQ(result.payload.count, 4);
    ASSERT_EQ(result.payload.offset, rtp::details::RTP_FIXED_HEADER_LEN);
}

// Packet with 0-length extension
TEST_F(RTPPacketParserTest, rtp_packet_with_empty_extension_test) {
    rtp::ParseStat stat;
    const auto pt = rtp::PayloadType::from_uint8(0).unwrap(); // PCMU
    rtp::PayloadMap map({std::make_pair(pt, rtp::PayloadMapItem{rtp::ClockRate(8000)})});
    auto result_rv =
        rtp::Packet::parse(
            util::ConstBinaryView(
                util::flat_vec<uint8_t>(
                    {
                        rtp_helpers::first_word(pt.value(), 0x1234, false, false, true),
                        helpers::uint32be(160),
                        helpers::uint32be(0xDEADBEEF),
                        rtp_helpers::extension_header(0xBEDE, 0x0),
                    }
                )
            ),
            map,
            stat);
    ASSERT_TRUE(result_rv.is_ok());
    auto result = result_rv.unwrap();
    EXPECT_EQ(stat.success.count(), 1);
    EXPECT_EQ(result.payload.count, 0);
    EXPECT_EQ(result.payload.offset, rtp::details::RTP_FIXED_HEADER_LEN + 4);
    ASSERT_TRUE(result.header.maybe_extension.is_some());
    EXPECT_EQ(result.header.maybe_extension.unwrap().profile_defined, 0xBEDE);
    EXPECT_EQ(result.header.maybe_extension.unwrap().data.offset, rtp::details::RTP_FIXED_HEADER_LEN + 4);
    EXPECT_EQ(result.header.maybe_extension.unwrap().data.count, 0);
}

TEST_F(RTPPacketParserTest, rtp_packet_with_1word_extension_test) {
    rtp::ParseStat stat;
    const auto pt = rtp::PayloadType::from_uint8(0).unwrap(); // PCMU
    rtp::PayloadMap map({std::make_pair(pt, rtp::PayloadMapItem{rtp::ClockRate(8000)})});
    auto result_rv =
        rtp::Packet::parse(
            util::ConstBinaryView(
                util::flat_vec<uint8_t>(
                    {
                        rtp_helpers::first_word(pt.value(), 0x1234, false, false, true),
                        helpers::uint32be(160),
                        helpers::uint32be(0xDEADBEEF),
                        rtp_helpers::extension_header(0xBEDE, 0x1),
                        {1, 2, 3, 4}
                    }
                )
            ),
            map,
            stat);
    ASSERT_TRUE(result_rv.is_ok());
    auto result = result_rv.unwrap();
    EXPECT_EQ(stat.success.count(), 1);
    EXPECT_EQ(result.payload.count, 0);
    EXPECT_EQ(result.payload.offset, rtp::details::RTP_FIXED_HEADER_LEN + 8);
    ASSERT_TRUE(result.header.maybe_extension.is_some());
    EXPECT_EQ(result.header.maybe_extension.unwrap().profile_defined, 0xBEDE);
    EXPECT_EQ(result.header.maybe_extension.unwrap().data.offset, rtp::details::RTP_FIXED_HEADER_LEN + 4);
    EXPECT_EQ(result.header.maybe_extension.unwrap().data.count, 4);
}

// Packet with 3 csrc
TEST_F(RTPPacketParserTest, packet_with_csrc_and_payload) {
    rtp::ParseStat stat;
    const auto pt = rtp::PayloadType::from_uint8(0).unwrap(); // PCMU
    const auto ssrc = rtp::SSRC::from_uint32(0xDEADBEEF);
    const uint32_t timestamp_value = 160;
    const uint16_t sequence_value = 0x1234;
    const std::vector<rtp::SSRC> csrcs = {
        rtp::SSRC::from_uint32(0x00C0FFEE),
        rtp::SSRC::from_uint32(0xCAFEDEAD),
        rtp::SSRC::from_uint32(0xBAADF00D)
    };

    rtp::PayloadMap map({std::make_pair(pt, rtp::PayloadMapItem{rtp::ClockRate(8000)})});
    auto result_rv =
        rtp::Packet::parse(
            util::ConstBinaryView(
                util::flat_vec<uint8_t>(
                    {
                        rtp_helpers::first_word(pt.value(), sequence_value, false, false, false, csrcs.size()),
                        helpers::uint32be(timestamp_value),
                        helpers::uint32be(ssrc.value()),
                        helpers::uint32be(csrcs[0].value()),
                        helpers::uint32be(csrcs[1].value()),
                        helpers::uint32be(csrcs[2].value()),
                        { 1, 2, 3, 4}
                    }
                )
            ),
            map,
            stat);
    ASSERT_TRUE(result_rv.is_ok());
    auto result = result_rv.unwrap();
    EXPECT_EQ(stat.success.count(), 1);
    EXPECT_TRUE(!result.header.marker);
    EXPECT_EQ(result.header.csrcs, csrcs);
    EXPECT_EQ(result.payload.count, 4);
    EXPECT_EQ(result.payload.offset, rtp::details::RTP_FIXED_HEADER_LEN + csrcs.size() * sizeof(uint32_t));
}

// ================================================================================
// Negative cases

// RTP Version is set to 3
TEST_F(RTPPacketParserTest, invalid_version_test) {
    rtp::ParseStat stat;
    const auto pt = rtp::PayloadType::from_uint8(0).unwrap(); // PCMU
    const auto ssrc = rtp::SSRC::from_uint32(0xDEADBEEF);
    const uint32_t timestamp_value = 160;
    const uint16_t sequence_value = 0x1234;

    rtp::PayloadMap map({std::make_pair(pt, rtp::PayloadMapItem{rtp::ClockRate(8000)})});
    auto firstw = rtp_helpers::first_word(pt.value(), sequence_value);
    firstw[0] = 0xC0;
    auto result_rv =
        rtp::Packet::parse(
            util::ConstBinaryView(
                util::flat_vec<uint8_t>(
                    {
                        firstw,
                        helpers::uint32be(timestamp_value),
                        helpers::uint32be(ssrc.value())
                    }
                )
            ),
            map,
            stat);
    ASSERT_TRUE(result_rv.is_err());
    ASSERT_EQ(stat.error.count(), 1);
    ASSERT_EQ(stat.invalid_version.count(), 1);
}

// Padding bit is set & empty payload (no padding length in last byte).
TEST_F(RTPPacketParserTest, invalid_padding_test) {
    rtp::ParseStat stat;
    const auto pt = rtp::PayloadType::from_uint8(0).unwrap(); // PCMU
    const auto ssrc = rtp::SSRC::from_uint32(0xDEADBEEF);
    const uint32_t timestamp_value = 160;
    const uint16_t sequence_value = 0x1234;

    rtp::PayloadMap map({std::make_pair(pt, rtp::PayloadMapItem{rtp::ClockRate(8000)})});
    auto result_rv =
        rtp::Packet::parse(
            util::ConstBinaryView(
                util::flat_vec<uint8_t>(
                    {
                        rtp_helpers::first_word(pt.value(), sequence_value, false, true),
                        helpers::uint32be(timestamp_value),
                        helpers::uint32be(ssrc.value())
                    }
                )
            ),
            map,
            stat);
    ASSERT_TRUE(result_rv.is_err());
    ASSERT_EQ(stat.error.count(), 1);
    ASSERT_EQ(stat.invalid_padding.count(), 1);
}

// Extension bit set but no room for extension header
TEST_F(RTPPacketParserTest, invalid_extension_no_header) {
    rtp::ParseStat stat;
    const auto pt = rtp::PayloadType::from_uint8(0).unwrap(); // PCMU
    rtp::PayloadMap map({std::make_pair(pt, rtp::PayloadMapItem{rtp::ClockRate(8000)})});
    auto result_rv =
        rtp::Packet::parse(
            util::ConstBinaryView(
                util::flat_vec<uint8_t>(
                    {
                        rtp_helpers::first_word(pt.value(), 0x1234, false, false, true),
                        helpers::uint32be(160),
                        helpers::uint32be(0xDEADBEEF)
                    }
                )
            ),
            map,
            stat);
    ASSERT_TRUE(result_rv.is_err());
    EXPECT_EQ(stat.error.count(), 1);
    EXPECT_EQ(stat.invalid_extension.count(), 1);
}

// Extension bit set / extension header length out of bound of packet
TEST_F(RTPPacketParserTest, invalid_extension_header_length) {
    rtp::ParseStat stat;
    const auto pt = rtp::PayloadType::from_uint8(0).unwrap(); // PCMU
    rtp::PayloadMap map({std::make_pair(pt, rtp::PayloadMapItem{rtp::ClockRate(8000)})});
    auto result_rv =
        rtp::Packet::parse(
            util::ConstBinaryView(
                util::flat_vec<uint8_t>(
                    {
                        rtp_helpers::first_word(pt.value(), 0x1234, false, false, true),
                        helpers::uint32be(160),
                        helpers::uint32be(0xDEADBEEF),
                        rtp_helpers::extension_header(0xBEDE, 0x1)
                    }
                )
            ),
            map,
            stat);
    ASSERT_TRUE(result_rv.is_err());
    EXPECT_EQ(stat.error.count(), 1);
    EXPECT_EQ(stat.invalid_extension.count(), 1);
}

// Packet with specified csrc but not enough room for it
TEST_F(RTPPacketParserTest, invalid_number_of_csrc) {
    rtp::ParseStat stat;
    const auto pt = rtp::PayloadType::from_uint8(0).unwrap(); // PCMU
    const auto ssrc = rtp::SSRC::from_uint32(0xDEADBEEF);
    const uint32_t timestamp_value = 160;
    const uint16_t sequence_value = 0x1234;
    rtp::PayloadMap map({std::make_pair(pt, rtp::PayloadMapItem{rtp::ClockRate(8000)})});
    auto result_rv =
        rtp::Packet::parse(
            util::ConstBinaryView(
                util::flat_vec<uint8_t>(
                    {
                        rtp_helpers::first_word(pt.value(), sequence_value, false, false, false, 4),
                        helpers::uint32be(timestamp_value),
                        helpers::uint32be(ssrc.value())
                    }
                )
            ),
            map,
            stat);
    ASSERT_TRUE(result_rv.is_err());
    EXPECT_EQ(stat.error.count(), 1);
    EXPECT_EQ(stat.invalid_csrc.count(), 1);
}

// Too short packet
TEST_F(RTPPacketParserTest, too_short_packet) {
    rtp::ParseStat stat;
    const auto pt = rtp::PayloadType::from_uint8(0).unwrap(); // PCMU
    rtp::PayloadMap map({std::make_pair(pt, rtp::PayloadMapItem{rtp::ClockRate(8000)})});
    EXPECT_FALSE(rtp::Packet::parse(util::ConstBinaryView(std::vector<uint8_t>{0x00}),  map, stat).is_ok());
    EXPECT_FALSE(rtp::Packet::parse(util::ConstBinaryView(std::vector<uint8_t>{0x00, 0x01}),  map, stat).is_ok());
    EXPECT_FALSE(rtp::Packet::parse(util::ConstBinaryView(std::vector<uint8_t>(rtp::details::RTP_FIXED_HEADER_LEN-1)), map, stat).is_ok());
    EXPECT_EQ(stat.error.count(), 3);
    EXPECT_EQ(stat.invalid_size.count(), 3);
}

}
