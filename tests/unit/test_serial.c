#include "test_serial.h"
#include "comm/serial.h"
#include "mocks/mock_mem.h"
#include "mocks/mock_sgdk.h"

#define VDP_MODE_REG_3 0xB
#define VDP_IE2 0x08
#define INT_MASK_LEVEL_ENABLE_ALL 1

static void init_serial(const u8 flags)
{
    expect_mem_write_u8(PORT2_SCTRL, flags);
    expect_mem_write_u8(PORT2_CTRL, CTRL_PCS_OUT);

    expect_value(__wrap_SYS_setInterruptMaskLevel, value, INT_MASK_LEVEL_ENABLE_ALL);
    expect_value(__wrap_VDP_setReg, reg, VDP_MODE_REG_3);
    expect_value(__wrap_VDP_setReg, value, VDP_IE2);

    expect_value(__wrap_VDP_getReg, reg, VDP_MODE_REG_3);
    will_return(__wrap_VDP_getReg, VDP_IE2);

    expect_any(__wrap_SYS_setExtIntCallback, CB);
    serial_init(flags);
}

int test_serial_setup(UNUSED void** state)
{
    init_serial(SCTRL_4800_BPS | SCTRL_SIN | SCTRL_SOUT | SCTRL_RINT);
    return 0;
}

void test_serial_inits(UNUSED void** state)
{
}

void test_serial_readyToReceive_when_not_ready(UNUSED void** state)
{
    expect_mem_read_u8(PORT2_SCTRL, 0);
    assert_false(serial_readyToReceive());
}

void test_serial_readyToReceive_when_ready(UNUSED void** state)
{
    expect_mem_read_u8(PORT2_SCTRL, SCTRL_RRDY);
    assert_true(serial_readyToReceive());
}

void test_serial_receive(UNUSED void** state)
{
    const u8 expected_data = 0x42;
    expect_mem_read_u8(PORT2_RX, expected_data);
    assert_int_equal(serial_receive(), expected_data);
}

void test_serial_sctrl(UNUSED void** state)
{
    const u8 expected_flags = SCTRL_4800_BPS | SCTRL_SIN | SCTRL_SOUT;
    expect_mem_read_u8(PORT2_SCTRL, expected_flags);
    assert_int_equal(serial_sctrl(), expected_flags);
}

void test_serial_sends(UNUSED void** state)
{
    const u8 data = 0x42;
    expect_mem_write_u8(PORT2_TX, data);
    serial_send(data);
}

void test_serial_readyToSend_when_not_ready(UNUSED void** state)
{
    expect_mem_read_u8(PORT2_SCTRL, SCTRL_TFUL);
    assert_false(serial_readyToSend());
}

void test_serial_readyToSend_when_ready(UNUSED void** state)
{
    expect_mem_read_u8(PORT2_SCTRL, 0);
    assert_true(serial_readyToSend());
}

void test_serial_sendWhenReady(UNUSED void** state)
{
    const u8 data = 0x42;
    expect_mem_read_u8(PORT2_SCTRL, 0); // Ready to send
    expect_mem_write_u8(PORT2_TX, data);
    serial_sendWhenReady(data);
}

void test_serial_sendWhenReady_waits_until_ready(UNUSED void** state)
{
    const u8 data = 0x42;
    expect_mem_read_u8(PORT2_SCTRL, SCTRL_TFUL); // Not ready
    expect_mem_read_u8(PORT2_SCTRL, SCTRL_TFUL); // Still not ready
    expect_mem_read_u8(PORT2_SCTRL, 0); // Finally ready
    expect_mem_write_u8(PORT2_TX, data);
    serial_sendWhenReady(data);
}

void test_serial_setReadyToReceiveCallback(UNUSED void** state)
{
    VoidCallback* callback = (VoidCallback*)0x1234;
    serial_setReadyToReceiveCallback(callback);
}
