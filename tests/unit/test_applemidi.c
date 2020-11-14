#include "cmocka_inc.h"
#include "applemidi.h"

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

static void test_applemidi_parses_notes_sysex_cc_in_one_packet(
    UNUSED void** state)
{
    char rtp_packet[] = { /* V P X CC M PT */ 0x80, 0x61,
        /* sequence number */ 0xe0, 0x19, /* timestamp */ 0x03, 0x31, 0xdd,
        0x6d, /* SSRC */ 0x09, 0x0f, 0x92, 0xe9,
        /* MIDI command section */ 0xc0, 0x61, /* cmds */ 0xb0, 0x00, 0x00,
        0x00, /* | */ 0xc0, 0x00, 0x00, /* | */ 0xf7, 0x00, 0xf0, 0x01,
        /* | */ 0xb0, 0x00, 0x00, 0x00, /* | */ 0xb1, 0x5d, 0x00, 0x06,
        /* | */ 0xb3, 0x00, 0x00, 0x00, /* A */ 0xc3, 0x00, 0x00, /* | */ 0xf7,
        0x00, 0xf0, 0x01, /* | */ 0xb3, 0x00, 0x00, 0x05, /* | */ 0xb4, 0x00,
        0x00, 0x00, /* | */ 0xc4, 0x00, 0x00, /* | */ 0xf7, 0x00, 0xf0, 0x01,
        /* | */ 0xb4, 0x00, 0x00, 0x05, /* | */ 0xb5, 0x00, 0x00, 0x00,
        /* | */ 0xc5, 0x00, 0x00, /* | */ 0xf7, 0x00, 0xf0, 0x01, /* B */ 0xb5,
        0x00, 0x00, 0x04, /* | */ 0xb6, 0x00, 0x00, 0x01, /* | */ 0xc6, 0x00,
        0x00, /* | */ 0xf7, 0x00, 0xf0, 0x00, /* | */ 0xb6, 0x00, 0x00,
        0x04, /* | */
        0xb7, 0x00, 0x00, 0x00, /* | */ 0xc7, 0x00, 0x00, /* | */ 0xf7, 0x00,
        0xf0, 0x01, /* | */ 0xb7, 0x00, 0x00, 0x00, /* | */ 0xe9, 0x00, 0x00 };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit_trio(0xb0, 0x00, 0x00);
    expect_midi_emit_duo(0xc0, 0x00);
    expect_midi_emit_trio(0xb0, 0x00, 0x00);
    expect_midi_emit_trio(0xb1, 0x5d, 0x00);
    expect_midi_emit_trio(0xb3, 0x00, 0x00);
    /* A */
    expect_midi_emit_duo(0xc3, 0x00);
    expect_midi_emit_trio(0xb3, 0x00, 0x00);
    expect_midi_emit_trio(0xb4, 0x00, 0x00);
    expect_midi_emit_duo(0xc4, 0x00);
    expect_midi_emit_trio(0xb4, 0x00, 0x00);
    expect_midi_emit_trio(0xb5, 0x00, 0x00);
    expect_midi_emit_duo(0xc5, 0x00);
    /* B */
    expect_midi_emit_trio(0xb5, 0x00, 0x00);
    expect_midi_emit_trio(0xb6, 0x00, 0x00);
    expect_midi_emit_duo(0xc6, 0x00);
    expect_midi_emit_trio(0xb6, 0x00, 0x00);
    expect_midi_emit_trio(0xb7, 0x00, 0x00);
    expect_midi_emit_duo(0xc7, 0x00);
    expect_midi_emit_trio(0xb7, 0x00, 0x00);
    expect_midi_emit_trio(0xe9, 0x00, 0x00);

    mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
    assert_int_equal(err, MW_ERR_NONE);
}

static void test_applemidi_ignores_middle_sysex_segments(UNUSED void** state)
{
    const u8 endings[] = { 0xF0, 0xF7 };
    const u8 cmd_length = 11;

    for (u8 i = 0; i < 2; i++) {
        const u8 ending = endings[i];
        print_message("Testing segment %d", i);

        char rtp_packet[] = { /* V P X CC M PT */ 0x80, 0x61,
            /* sequence number */ 0xe0, 0x19, /* timestamp */ 0x03, 0x31, 0xdd,
            0x6d, /* SSRC */ 0x09, 0x0f, 0x92, 0xe9,
            /* MIDI command section */ 0xc0, cmd_length, /* cmds */ 0x90, 0x60,
            0x61, 0x00, /* | */ 0xF7, 0x00, ending, 0x01,
            /* | */ 0x90, 0x60, 0x61 };

        size_t len = sizeof(rtp_packet);

        expect_midi_emit_trio(0x90, 0x60, 0x61);
        expect_midi_emit_trio(0x90, 0x60, 0x61);

        mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
        assert_int_equal(err, MW_ERR_NONE);
    }
}

static void test_applemidi_processes_multiple_sysex_segments(
    UNUSED void** state)
{
    const u8 cmd_length = 12;
    char rtp_packet[] = { /* V P X CC M PT */ 0x80, 0x61,
        /* sequence number */ 0xe0, 0x19, /* timestamp */ 0x03, 0x31, 0xdd,
        0x6d, /* SSRC */ 0x09, 0x0f, 0x92, 0xe9,
        /* MIDI command section */ 0xc0, cmd_length, /* cmds */ 0xF0, 0x00,
        0xF7, 0x00, /* | */ 0xF0, 0x01, 0xF7, 0x01,
        /* | */ 0xF0, 0x02, 0xF7, 0x00 };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit_trio(0xF0, 0x00, 0xF7);
    expect_midi_emit_trio(0xF0, 0x01, 0xF7);
    expect_midi_emit_trio(0xF0, 0x02, 0xF7);
    //  expect_midi_emit(0x00);
    //  expect_midi_emit(0x00);
    // expect_midi_emit(0x00);
    // expect_midi_emit(0x00);

    mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
    assert_int_equal(err, MW_ERR_NONE);
}

static void test_applemidi_processes_ccs(UNUSED void** state)
{
    char rtp_packet[] = { /* V P X CC M PT */ 0x80, 0x61,
        /* sequence number */ 0xf3, 0x86, /* timestamp */ 0x08, 0xbe, 0x9f,
        0x2b, /* SSRC */ 0x09, 0x0f, 0x92, 0xe9,
        /* MIDI command section */ 0xc0, 0x34, /* cmds */ 0xb1, 0x00, 0x00,
        0x00, /* | */ 0xc1, 0x00, 0x00, /* | */ 0xf7, 0x00, 0xf0, 0x01,
        /* | */ 0xb1, 0x00, 0x00, 0x07, /* | */ 0xc1, 0x0b, 0x00, /* | */ 0xf7,
        0x00, 0xf0, 0x00, /* | */ 0xb1, 0x5d, 0x14, 0x00, /* | */ 0x5b, 0x64,
        0x07, /* | */ 0x07, 0x69, 0x00, /* | */ 0x0b, 0x7f, 0x00, /* A */ 0x64,
        0x01, 0x00, /* | */ 0x65, 0x00, 0x00, /* | */ 0x06, 0x20, 0x00,
        /* | */ 0x26, 0x08, 0x00, /* | */ 0x64, 0x00, 0x00, /* | */ 0x65,
        0x00 };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit_trio(0xb1, 0x00, 0x00);
    expect_midi_emit_duo(0xc1, 0x00);
    expect_midi_emit_trio(0xb1, 0x00, 0x00);
    expect_midi_emit_duo(0xc1, 0x0b);
    expect_midi_emit_trio(0xb1, 0x5d, 0x14);
    expect_midi_emit_trio(0xb1, 0x5b, 0x64);
    expect_midi_emit_trio(0xb1, 0x07, 0x69);
    expect_midi_emit_trio(0xb1, 0x0b, 0x7f);
    /* A */
    expect_midi_emit_trio(0xb1, 0x64, 0x01);
    expect_midi_emit_trio(0xb1, 0x65, 0x00);
    expect_midi_emit_trio(0xb1, 0x06, 0x20);
    expect_midi_emit_trio(0xb1, 0x26, 0x08);
    expect_midi_emit_trio(0xb1, 0x64, 0x00);
    expect_midi_emit_trio(0xb1, 0x65, 0x00);

    mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
    assert_int_equal(err, MW_ERR_NONE);
}

static void test_applemidi_sets_last_sequence_number(UNUSED void** state)
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

    u16 seqNum = applemidi_lastSequenceNumber();
    assert_int_equal(seqNum, 0x8c24);
}

static void test_applemidi_sends_receiver_feedback(UNUSED void** state)
{
    char rtp_packet[1024] = { /* V P X CC M PT */ 0x80, 0x61,
        /* sequence number */ 0x00, 0x01,
        /* timestamp */ 0x00, 0x58, 0xbb, 0x40, /* SSRC */ 0xac, 0x67, 0xe1,
        0x08, /* MIDI command section */ 0xC0, 0x03, 0x90, 0x48, 0x6f };

    size_t len = sizeof(rtp_packet);

    expect_midi_emit(0x90);
    expect_midi_emit(0x48);
    expect_midi_emit(0x6f);

    mw_err err = applemidi_processSessionMidiPacket(rtp_packet, len);
    assert_int_equal(err, MW_ERR_NONE);

    const u8 receiverFeedbackPacket[] = { 0xff, 0xff, 'R', 'S',
        /* SSRC */ 0x9E, 0x91, 0x51, 0x50, /* sequence number */
        0x00, 0x01, 0x00, 0x00 };
    expect_value(__wrap_comm_megawifi_send, ch, CH_CONTROL_PORT);
    expect_memory(__wrap_comm_megawifi_send, data, receiverFeedbackPacket,
        sizeof(receiverFeedbackPacket));
    expect_value(
        __wrap_comm_megawifi_send, len, sizeof(receiverFeedbackPacket));

    err = applemidi_sendReceiverFeedback();
    assert_int_equal(err, MW_ERR_NONE);
}
