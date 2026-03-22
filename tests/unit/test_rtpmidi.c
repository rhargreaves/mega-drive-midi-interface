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
