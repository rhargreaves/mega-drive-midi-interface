#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "unused.h"
#include <cmocka.h>
#include <stdio.h>

#include "applemidi.h"
#include "asserts.h"

static int test_applemidi_setup(UNUSED void** state)
{
    return 0;
}

static void test_applemidi_parses_rtpmidi_packet_with_single_midi_event(
    UNUSED void** state)
{
    char rtp_packet[1024]
        = { /* V P X CC M PT */ 0x80, 0x61, /* sequence number */ 0x8c, 0x24,
              /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67,
              0xe1, 0x08, /* MIDI command section */ 0x03, 0x90, 0x48, 0x6f };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);

    mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
    assert_int_equal(err, MW_ERR_NONE);
}

static void test_applemidi_parses_rtpmidi_packet_with_single_2_byte_midi_event(
    UNUSED void** state)
{
    const u8 statuses[] = { 0xC0, 0xD0, 0xF1, 0xF3 };

    for (u16 i = 0; i < sizeof(statuses); i++) {
        u8 status = statuses[i];
        char rtp_packet[1024] = { /* V P X CC M PT */ 0x80, 0x61,
            /* sequence number */ 0x8c, 0x24,
            /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67, 0xe1,
            0x08, /* MIDI command section */ 0x02, status, 0x01 };
        size_t len = sizeof(rtp_packet);

        expect_midi_emit(status);
        expect_midi_emit(0x01);

        mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
        assert_int_equal(err, MW_ERR_NONE);
    }
}

static void
test_applemidi_parses_rtpmidi_packet_with_multiple_2_byte_midi_events(
    UNUSED void** state)
{
    const u8 statuses[] = { 0xC0, 0xD0, 0xF1, 0xF3 };

    for (u16 i = 0; i < sizeof(statuses); i++) {
        u8 status = statuses[i];
        char rtp_packet[1024] = { /* V P X CC M PT */ 0x80, 0x61,
            /* sequence number */ 0x8c, 0x24,
            /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67, 0xe1,
            0x08, /* MIDI command section */ 0x05, status, 0x01, 0x00, status,
            0x01 };
        size_t len = sizeof(rtp_packet);

        expect_midi_emit(status);
        expect_midi_emit(0x01);
        expect_midi_emit(status);
        expect_midi_emit(0x01);

        mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
        assert_int_equal(err, MW_ERR_NONE);
    }
}

static void
test_applemidi_parses_rtpmidi_packet_with_single_midi_event_long_header(
    UNUSED void** state)
{
    char rtp_packet[1024] = { /* V P X CC M PT */ 0x80, 0x61,
        /* sequence number */ 0x8c, 0x24,
        /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67, 0xe1,
        0x08, /* MIDI command section */ 0xC0, 0x03, 0x90, 0x48, 0x6f };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);

    mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
    assert_int_equal(err, MW_ERR_NONE);
}

static void test_applemidi_parses_rtpmidi_packet_with_two_midi_events(
    UNUSED void** state)
{
    char rtp_packet[1024]
        = { /* V P X CC M PT */ 0x80, 0x61, /* sequence number */ 0x8c, 0x24,
              /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67,
              0xe1, 0x08, /* MIDI command section */ 0x06, 0x90, 0x48, 0x6f,
              0x00, 0x51, 0x6f };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);

    expect_midi_emit(0x90);
    expect_midi_emit(0x51);
    expect_midi_emit(0x6f);

    mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
    assert_int_equal(err, MW_ERR_NONE);
}

static void test_applemidi_parses_rtpmidi_packet_with_multiple_midi_events(
    UNUSED void** state)
{
    char rtp_packet[1024]
        = { /* V P X CC M PT */ 0x80, 0x61, /* sequence number */ 0x8c, 0x24,
              /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67,
              0xe1, 0x08, /* MIDI command section */ 0x09, 0x90, 0x48, 0x6f,
              0x00, 0x51, 0x6f, 0x00, 0x48, 0x6f };

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

    mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
    assert_int_equal(err, MW_ERR_NONE);
}

static void test_applemidi_parses_rtpmidi_packet_with_sysex(UNUSED void** state)
{
    char rtpPacket[1024] = { /* V P X CC M PT */ 0x80, 0x61,
        /* sequence number */ 0x8c, 0x24,
        /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67, 0xe1,
        0x08, /* MIDI command section */ 0x05, 0xF0, 0x12, 0x34, 0x56, 0xF7 };

    size_t len = sizeof(rtpPacket);

    expect_midi_emit(0xF0);
    expect_midi_emit(0x12);
    expect_midi_emit(0x34);
    expect_midi_emit(0x56);
    expect_midi_emit(0xF7);

    mw_err err = applemidi_processSessionMidiPacket(rtpPacket, len);
    assert_int_equal(err, MW_ERR_NONE);
}

static void
test_applemidi_parses_rtpmidi_packet_with_multiple_different_midi_events(
    UNUSED void** state)
{
    char rtp_packet[1024]
        = { /* V P X CC M PT */ 0x80, 0x61, /* sequence number */ 0x8c, 0x24,
              /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67,
              0xe1, 0x08, /* MIDI command section */ 0x0A, 0x90, 0x48, 0x6f,
              0x00, 0x51, 0x6f, 0x00, 0x80, 0x48, 0x6f };

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

    mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
    assert_int_equal(err, MW_ERR_NONE);
}
