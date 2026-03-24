#include "test_applemidi.h"
#include "comm/applemidi.h"
#include "mocks/mock_sgdk.h"

int test_applemidi_setup(UNUSED void** state)
{
    return 0;
}

void test_applemidi_sends_receiver_feedback(UNUSED void** state)
{
    applemidi_updateLastSeqNum(0x0001);

    const u8 receiverFeedbackPacket[] = { 0xff, 0xff, 'R', 'S',
        /* SSRC */ 0x9E, 0x91, 0x51, 0x50,
        /* sequence number */ 0x00, 0x01, 0x00, 0x00 };
    expect_value(__wrap_comm_megawifi_send, ch, CH_CONTROL_PORT);
    expect_memory(
        __wrap_comm_megawifi_send, data, receiverFeedbackPacket, sizeof(receiverFeedbackPacket));
    expect_value(__wrap_comm_megawifi_send, len, sizeof(receiverFeedbackPacket));

    mw_err err = applemidi_sendReceiverFeedback();
    assert_int_equal(err, MW_ERR_NONE);
}

void test_applemidi_accepts_receiver_feedback_packet(UNUSED void** state)
{
    char receiverFeedbackPacket[] = { 0xff, 0xff, 'R', 'S',
        /* SSRC */ 0x9E, 0x91, 0x51, 0x50,
        /* sequence number */ 0x00, 0x01, 0x00, 0x00 };
    applemidi_control_event event = APPLEMIDI_CTRL_EVENT_NONE;

    midi_pkt_result result = applemidi_processSessionControlPacket(
        receiverFeedbackPacket, sizeof(receiverFeedbackPacket), &event);

    assert_int_equal(result, MIDI_PKT_OK);
    assert_int_equal(event, APPLEMIDI_CTRL_EVENT_NONE);
}

void test_applemidi_emits_session_end_event(UNUSED void** state)
{
    char sessionEndPacket[] = { 0xff, 0xff, 'B', 'Y',
        /* protocol version */ 0x00, 0x00, 0x00, 0x02,
        /* init token */ 0x00, 0x00, 0x00, 0x00,
        /* SSRC */ 0x11, 0x22, 0x33, 0x44 };
    applemidi_control_event event = APPLEMIDI_CTRL_EVENT_NONE;

    midi_pkt_result result
        = applemidi_processSessionControlPacket(sessionEndPacket, sizeof(sessionEndPacket), &event);

    assert_int_equal(result, MIDI_PKT_OK);
    assert_int_equal(event, APPLEMIDI_CTRL_EVENT_SESSION_END);
}
