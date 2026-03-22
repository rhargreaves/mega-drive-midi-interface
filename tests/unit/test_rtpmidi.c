#include "test_rtpmidi.h"
#include "comm/rtpmidi.h"
#include "mocks/mock_midi.h"

void test_rtpmidi_does_not_read_beyond_buffer_length(UNUSED void** state)
{
    char rtp_packet[] = { /* V P X CC M PT */ 0x80, 0x61,
        /* sequence number */ 0x8c, 0x24,
        /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67, 0xe1, 0x08,
        /* MIDI command section */ 0x05, 0xF0, 0x01, 0xF7, 0x00, 0x00,
        /* should not emit this byte */ 0x00 };

    size_t fullLen = sizeof(rtp_packet);
    size_t truncatedLen = fullLen - 2;

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, truncatedLen);
    assert_int_equal(result, MIDI_PKT_RTP_LENGTH_MISMATCH);
}

void test_rtpmidi_returns_when_header_too_short(UNUSED void** state)
{
    char rtp_packet[] = { 0x80, 0x61, 0x00, 0x01, 0x00 };

    size_t len = sizeof(rtp_packet);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
    assert_int_equal(result, MIDI_PKT_RTP_HEADER_TOO_SHORT);
}

void test_rtpmidi_updates_applemidi_last_sequence_number(UNUSED void** state)
{
    applemidi_updateLastSeqNum(0);

    char rtp_packet[] = { 0x80, 0x61, 0x8c, 0x24, 0x00, 0x58, 0xbb, 0x40, 0xac, 0x67, 0xe1, 0x08,
        0x03, 0x90, 0x48, 0x6f };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
    assert_int_equal(result, MIDI_PKT_OK);
    assert_int_equal(applemidi_lastSequenceNumber(), 0x8c24);
}

void test_rtpmidi_parses_packet_with_single_midi_event(UNUSED void** state)
{
    char rtp_packet[1024] = { 0x80, 0x61, 0x8c, 0x24, 0x00, 0x58, 0xbb, 0x40, 0xac, 0x67, 0xe1, 0x08,
        0x03, 0x90, 0x48, 0x6f };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
    assert_int_equal(result, MIDI_PKT_OK);
}

void test_rtpmidi_parses_packet_with_single_2_byte_midi_event(UNUSED void** state)
{
    const u8 statuses[] = { 0xC0, 0xD0, 0xF1, 0xF3 };

    for (u16 i = 0; i < sizeof(statuses); i++) {
        u8 status = statuses[i];
        char rtp_packet[1024] = { 0x80, 0x61, 0x8c, 0x24, 0x00, 0x58, 0xbb, 0x40, 0xac, 0x67, 0xe1,
            0x08, 0x02, status, 0x01 };
        size_t len = sizeof(rtp_packet);

        expect_midi_emit(status);
        expect_midi_emit(0x01);

        midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
        assert_int_equal(result, MIDI_PKT_OK);
    }
}

void test_rtpmidi_parses_packet_with_multiple_2_byte_midi_events(UNUSED void** state)
{
    const u8 statuses[] = { 0xC0, 0xD0, 0xF1, 0xF3 };

    for (u16 i = 0; i < sizeof(statuses); i++) {
        u8 status = statuses[i];
        char rtp_packet[1024] = { 0x80, 0x61, 0x8c, 0x24, 0x00, 0x58, 0xbb, 0x40, 0xac, 0x67, 0xe1,
            0x08, 0x05, status, 0x01, 0x00, status, 0x01 };
        size_t len = sizeof(rtp_packet);

        expect_midi_emit(status);
        expect_midi_emit(0x01);
        expect_midi_emit(status);
        expect_midi_emit(0x01);

        midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
        assert_int_equal(result, MIDI_PKT_OK);
    }
}

void test_rtpmidi_parses_packet_with_single_midi_event_long_header(UNUSED void** state)
{
    char rtp_packet[1024] = { 0x80, 0x61, 0x8c, 0x24, 0x00, 0x58, 0xbb, 0x40, 0xac, 0x67, 0xe1, 0x08,
        0xC0, 0x03, 0x90, 0x48, 0x6f };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
    assert_int_equal(result, MIDI_PKT_OK);
}

void test_rtpmidi_parses_packet_with_two_midi_events(UNUSED void** state)
{
    char rtp_packet[1024] = { 0x80, 0x61, 0x8c, 0x24, 0x00, 0x58, 0xbb, 0x40, 0xac, 0x67, 0xe1, 0x08,
        0x06, 0x90, 0x48, 0x6f, 0x00, 0x51, 0x7c };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);
    expect_midi_emit(0x90);
    expect_midi_emit(0x51);
    expect_midi_emit(0x7c);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
    assert_int_equal(result, MIDI_PKT_OK);
}

void test_rtpmidi_parses_packet_with_multiple_midi_events(UNUSED void** state)
{
    char rtp_packet[1024] = { 0x80, 0x61, 0x8c, 0x24, 0x00, 0x58, 0xbb, 0x40, 0xac, 0x67, 0xe1, 0x08,
        0x09, 0x90, 0x48, 0x6f, 0x00, 0x51, 0x6f, 0x00, 0x48, 0x6f };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);
    expect_midi_emit(0x90);
    expect_midi_emit(0x51);
    expect_midi_emit(0x6f);
    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
    assert_int_equal(result, MIDI_PKT_OK);
}

void test_rtpmidi_parses_packet_with_sysex(UNUSED void** state)
{
    char rtpPacket[1024] = { 0x80, 0x61, 0x8c, 0x24, 0x00, 0x58, 0xbb, 0x40, 0xac, 0x67, 0xe1, 0x08,
        0x05, 0xF0, 0x12, 0x34, 0x56, 0xF7 };

    size_t len = sizeof(rtpPacket);

    expect_midi_emit(0xF0);
    expect_midi_emit(0x12);
    expect_midi_emit(0x34);
    expect_midi_emit(0x56);
    expect_midi_emit(0xF7);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtpPacket, len);
    assert_int_equal(result, MIDI_PKT_OK);
}

void test_rtpmidi_parses_packet_with_sysex_ending_with_F0(UNUSED void** state)
{
    char rtpPacket[1024] = { 0x80, 0x61, 0x8c, 0x24, 0x00, 0x58, 0xbb, 0x40, 0xac, 0x67, 0xe1, 0x08,
        0x05, 0xF0, 0x12, 0x34, 0x56, 0xF0 };

    size_t len = sizeof(rtpPacket);

    expect_midi_emit(0xF0);
    expect_midi_emit(0x12);
    expect_midi_emit(0x34);
    expect_midi_emit(0x56);
    expect_midi_emit(0xF7);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtpPacket, len);
    assert_int_equal(result, MIDI_PKT_OK);
}

void test_rtpmidi_parses_packet_with_sysex_with_0xF7_at_end(UNUSED void** state)
{
    char rtpPacket[1024] = { 0x80, 0x61, 0x8c, 0x24, 0x00, 0x58, 0xbb, 0x40, 0xac, 0x67, 0xe1, 0x08,
        0x05, 0xF0, 0x12, 0x34, 0x56, 0xF7, 0xF7 };

    size_t len = sizeof(rtpPacket);

    expect_midi_emit(0xF0);
    expect_midi_emit(0x12);
    expect_midi_emit(0x34);
    expect_midi_emit(0x56);
    expect_midi_emit(0xF7);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtpPacket, len);
    assert_int_equal(result, MIDI_PKT_OK);
}

void test_rtpmidi_parses_packet_with_multiple_different_midi_events(UNUSED void** state)
{
    char rtp_packet[1024] = { 0x80, 0x61, 0x8c, 0x24, 0x00, 0x58, 0xbb, 0x40, 0xac, 0x67, 0xe1, 0x08,
        0x0A, 0x90, 0x48, 0x6f, 0x00, 0x51, 0x6f, 0x00, 0x80, 0x48, 0x6f };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);
    expect_midi_emit(0x90);
    expect_midi_emit(0x51);
    expect_midi_emit(0x6f);
    expect_midi_emit(0x80);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
    assert_int_equal(result, MIDI_PKT_OK);
}

void test_rtpmidi_parses_packet_with_system_reset(UNUSED void** state)
{
    char rtp_packet[1024] = { 0x80, 0x61, 0x8c, 0x24, 0x00, 0x58, 0xbb, 0x40, 0xac, 0x67, 0xe1, 0x08,
        0x01, 0xff };

    size_t len = sizeof(rtp_packet);
    expect_midi_emit(0xff);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
    assert_int_equal(result, MIDI_PKT_OK);
}

void test_rtpmidi_parses_notes_sysex_cc_in_one_packet(UNUSED void** state)
{
    char rtp_packet[] = { 0x80, 0x61, 0xe0, 0x19, 0x03, 0x31, 0xdd, 0x6d, 0x09, 0x0f, 0x92, 0xe9,
        0xc0, 0x61, 0xb0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xf7, 0x00, 0xf0, 0x01, 0xb0, 0x00,
        0x00, 0x00, 0xb1, 0x5d, 0x00, 0x06, 0xb3, 0x00, 0x00, 0x00, 0xc3, 0x00, 0x00, 0xf7, 0x00,
        0xf0, 0x01, 0xb3, 0x00, 0x00, 0x05, 0xb4, 0x00, 0x00, 0x00, 0xc4, 0x00, 0x00, 0xf7, 0x00,
        0xf0, 0x01, 0xb4, 0x00, 0x00, 0x05, 0xb5, 0x00, 0x00, 0x00, 0xc5, 0x00, 0x00, 0xf7, 0x00,
        0xf0, 0x01, 0xb5, 0x00, 0x00, 0x04, 0xb6, 0x00, 0x00, 0x01, 0xc6, 0x00, 0x00, 0xf7, 0x00,
        0xf0, 0x00, 0xb6, 0x00, 0x00, 0x04, 0xb7, 0x00, 0x00, 0x00, 0xc7, 0x00, 0x00, 0xf7, 0x00,
        0xf0, 0x01, 0xb7, 0x00, 0x00, 0x00, 0xe9, 0x00, 0x00 };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit_trio(0xb0, 0x00, 0x00);
    expect_midi_emit_duo(0xc0, 0x00);
    expect_midi_emit_trio(0xb0, 0x00, 0x00);
    expect_midi_emit_trio(0xb1, 0x5d, 0x00);
    expect_midi_emit_trio(0xb3, 0x00, 0x00);
    expect_midi_emit_duo(0xc3, 0x00);
    expect_midi_emit_trio(0xb3, 0x00, 0x00);
    expect_midi_emit_trio(0xb4, 0x00, 0x00);
    expect_midi_emit_duo(0xc4, 0x00);
    expect_midi_emit_trio(0xb4, 0x00, 0x00);
    expect_midi_emit_trio(0xb5, 0x00, 0x00);
    expect_midi_emit_duo(0xc5, 0x00);
    expect_midi_emit_trio(0xb5, 0x00, 0x00);
    expect_midi_emit_trio(0xb6, 0x00, 0x00);
    expect_midi_emit_duo(0xc6, 0x00);
    expect_midi_emit_trio(0xb6, 0x00, 0x00);
    expect_midi_emit_trio(0xb7, 0x00, 0x00);
    expect_midi_emit_duo(0xc7, 0x00);
    expect_midi_emit_trio(0xb7, 0x00, 0x00);
    expect_midi_emit_trio(0xe9, 0x00, 0x00);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
    assert_int_equal(result, MIDI_PKT_OK);
}

void test_rtpmidi_ignores_middle_sysex_segments(UNUSED void** state)
{
    const u8 endings[] = { 0xF0, 0xF7 };
    const u8 cmd_length = 11;

    for (u8 i = 0; i < 2; i++) {
        const u8 ending = endings[i];
        char rtp_packet[] = { 0x80, 0x61, 0xe0, 0x19, 0x03, 0x31, 0xdd, 0x6d, 0x09, 0x0f, 0x92, 0xe9,
            0xc0, cmd_length, 0x90, 0x60, 0x61, 0x00, 0xF7, 0x00, ending, 0x01, 0x90, 0x60, 0x61 };

        size_t len = sizeof(rtp_packet);
        expect_midi_emit_trio(0x90, 0x60, 0x61);
        expect_midi_emit_trio(0x90, 0x60, 0x61);

        midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
        assert_int_equal(result, MIDI_PKT_OK);
    }
}

void test_rtpmidi_processes_multiple_sysex_segments(UNUSED void** state)
{
    const u8 cmd_length = 12;
    char rtp_packet[] = { 0x80, 0x61, 0xe0, 0x19, 0x03, 0x31, 0xdd, 0x6d, 0x09, 0x0f, 0x92, 0xe9,
        0xc0, cmd_length, 0xF0, 0x00, 0xF7, 0x00, 0xF0, 0x01, 0xF7, 0x01, 0xF0, 0x02, 0xF7, 0x00 };

    size_t len = sizeof(rtp_packet);
    expect_midi_emit_trio(0xF0, 0x00, 0xF7);
    expect_midi_emit_trio(0xF0, 0x01, 0xF7);
    expect_midi_emit_trio(0xF0, 0x02, 0xF7);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
    assert_int_equal(result, MIDI_PKT_OK);
}

void test_rtpmidi_processes_ccs(UNUSED void** state)
{
    char rtp_packet[] = { 0x80, 0x61, 0xf3, 0x86, 0x08, 0xbe, 0x9f, 0x2b, 0x09, 0x0f, 0x92, 0xe9,
        0xc0, 0x34, 0xb1, 0x00, 0x00, 0x00, 0xc1, 0x00, 0x00, 0xf7, 0x00, 0xf0, 0x01, 0xb1, 0x00,
        0x00, 0x07, 0xc1, 0x0b, 0x00, 0xf7, 0x00, 0xf0, 0x00, 0xb1, 0x5d, 0x14, 0x00, 0x5b, 0x64,
        0x07, 0x07, 0x69, 0x00, 0x0b, 0x7f, 0x00, 0x64, 0x01, 0x00, 0x65, 0x00, 0x00, 0x06, 0x20,
        0x00, 0x26, 0x08, 0x00, 0x64, 0x00, 0x00, 0x65, 0x00 };

    size_t len = sizeof(rtp_packet);
    expect_midi_emit_trio(0xb1, 0x00, 0x00);
    expect_midi_emit_duo(0xc1, 0x00);
    expect_midi_emit_trio(0xb1, 0x00, 0x00);
    expect_midi_emit_duo(0xc1, 0x0b);
    expect_midi_emit_trio(0xb1, 0x5d, 0x14);
    expect_midi_emit_trio(0xb1, 0x5b, 0x64);
    expect_midi_emit_trio(0xb1, 0x07, 0x69);
    expect_midi_emit_trio(0xb1, 0x0b, 0x7f);
    expect_midi_emit_trio(0xb1, 0x64, 0x01);
    expect_midi_emit_trio(0xb1, 0x65, 0x00);
    expect_midi_emit_trio(0xb1, 0x06, 0x20);
    expect_midi_emit_trio(0xb1, 0x26, 0x08);
    expect_midi_emit_trio(0xb1, 0x64, 0x00);
    expect_midi_emit_trio(0xb1, 0x65, 0x00);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
    assert_int_equal(result, MIDI_PKT_OK);
}

void test_rtpmidi_sets_last_sequence_number(UNUSED void** state)
{
    applemidi_updateLastSeqNum(0);
    char rtp_packet[1024] = { 0x80, 0x61, 0x8c, 0x24, 0x00, 0x58, 0xbb, 0x40, 0xac, 0x67, 0xe1, 0x08,
        0xC0, 0x03, 0x90, 0x48, 0x6f };

    size_t len = sizeof(rtp_packet);
    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
    assert_int_equal(result, MIDI_PKT_OK);
    assert_int_equal(applemidi_lastSequenceNumber(), 0x8c24);
}

void test_rtpmidi_does_not_read_beyond_length(UNUSED void** state)
{
    u8 length = 4;
    char rtp_packet[1024] = { 0x80, 0x61, 0x8c, 0x24, 0x00, 0x58, 0xbb, 0x40, 0xac, 0x67, 0xe1, 0x08,
        0xC0, length, 0xF0, 0x01, 0xF7, 0x00, 0x00 };

    size_t len = sizeof(rtp_packet);
    expect_midi_emit_trio(0xF0, 0x01, 0xF7);

    midi_pkt_result result = rtpmidi_processRtpMidiPacket(rtp_packet, len);
    assert_int_equal(result, MIDI_PKT_OK);
    assert_int_equal(applemidi_lastSequenceNumber(), 0x8c24);
}

void test_rtpmidi_pack_packet_with_short_header(UNUSED void** state)
{
    u8 midiData[] = { 0x90, 0x48, 0x6f };
    u8 buffer[64] = { 0 };

    u16 packedLength = rtpmidi_packRtpMidiPacket(midiData, sizeof(midiData), buffer, sizeof(buffer));

    assert_int_equal(packedLength, RTP_MIDI_HEADER_LEN + 1 + sizeof(midiData));
    assert_int_equal(buffer[0], 0x80);
    assert_int_equal(buffer[1], 0xE1);
    assert_int_equal(buffer[12], sizeof(midiData));
    assert_memory_equal(&buffer[13], midiData, sizeof(midiData));
}

void test_rtpmidi_pack_packet_with_long_header(UNUSED void** state)
{
    u8 midiData[16] = { 0 };
    u8 buffer[64] = { 0 };

    u16 packedLength = rtpmidi_packRtpMidiPacket(midiData, sizeof(midiData), buffer, sizeof(buffer));

    assert_int_equal(packedLength, RTP_MIDI_HEADER_LEN + 2 + sizeof(midiData));
    assert_int_equal(buffer[0], 0x80);
    assert_int_equal(buffer[1], 0xE1);
    assert_int_equal(buffer[12], 0x80);
    assert_int_equal(buffer[13], sizeof(midiData));
    assert_memory_equal(&buffer[14], midiData, sizeof(midiData));
}

void test_rtpmidi_pack_packet_returns_zero_when_buffer_too_small(UNUSED void** state)
{
    u8 midiData[] = { 0x90, 0x48, 0x6f };
    u8 buffer[8] = { 0 };

    u16 packedLength = rtpmidi_packRtpMidiPacket(midiData, sizeof(midiData), buffer, sizeof(buffer));

    assert_int_equal(packedLength, 0);
}
