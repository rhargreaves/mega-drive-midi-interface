#include "test_rtpmidi.h"
#include "comm/rtpmidi.h"
#include "mocks/mock_midi.h"
#include "mocks/mock_sgdk.h"

void test_rtpmidi_does_not_read_beyond_buffer_length(UNUSED void** state)
{
    u8 midiLength = 0x05;
    char rtp_packet[] = { /* V P X CC M PT */ 0x80, 0x61,
        /* sequence number */ 0x8c, 0x24,
        /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67, 0xe1, 0x08,
        /* MIDI command section */ 0x00, midiLength, 0xF0, 0x01, 0xF7, 0x00,
        /* should not emit this byte */ 0x00 };

    size_t fullLen = sizeof(rtp_packet);
    size_t truncatedLen = fullLen - 1;

    u16 lastSeqNum = 0;
    mw_err err = rtpmidi_processRtpMidiPacket(rtp_packet, truncatedLen, &lastSeqNum);
    assert_int_equal(err, MW_ERR_NONE);
}

void test_rtpmidi_returns_when_header_too_short(UNUSED void** state)
{
    char rtp_packet[] = { 0x80, 0x61, 0x00, 0x01, 0x00 };

    size_t len = sizeof(rtp_packet);

    u16 lastSeqNum = 0;
    mw_err err = rtpmidi_processRtpMidiPacket(rtp_packet, len, &lastSeqNum);
    assert_int_equal(err, MW_ERR_NONE);
}
