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
        /* SSRC */ 0x9E, 0x91, 0x51, 0x50, /* sequence number */
        0x00, 0x01, 0x00, 0x00 };
    expect_value(__wrap_comm_megawifi_send, ch, CH_CONTROL_PORT);
    expect_memory(
        __wrap_comm_megawifi_send, data, receiverFeedbackPacket, sizeof(receiverFeedbackPacket));
    expect_value(__wrap_comm_megawifi_send, len, sizeof(receiverFeedbackPacket));

    mw_err err = applemidi_sendReceiverFeedback();
    assert_int_equal(err, MW_ERR_NONE);
}
