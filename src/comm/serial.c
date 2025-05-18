#include "serial.h"
#include "mem.h"

#define VDP_MODE_REG_3 0xB
#define VDP_IE2 0x08
#define INT_MASK_LEVEL_ENABLE_ALL 1

static VoidCallback* readReadyCallback;

static void ext_int_callback(void)
{
    readReadyCallback();
}

static void set_sctrl(u8 value)
{
    mem_write_u8(PORT2_SCTRL, value);
}

static void set_ctrl(u8 value)
{
    mem_write_u8(PORT2_CTRL, value);
}

u8 serial_sctrl(void)
{
    return mem_read_u8(PORT2_SCTRL);
}

bool serial_readyToReceive(void)
{
    return mem_read_u8(PORT2_SCTRL) & SCTRL_RRDY;
}

u8 serial_receive(void)
{
    return mem_read_u8(PORT2_RX);
}

void serial_setReadyToReceiveCallback(VoidCallback* cb)
{
    readReadyCallback = cb;
}

void serial_init(u8 sctrlFlags)
{
    set_sctrl(sctrlFlags);
    set_ctrl(CTRL_PCS_OUT);
    if (sctrlFlags & SCTRL_RINT) {
        SYS_setInterruptMaskLevel(INT_MASK_LEVEL_ENABLE_ALL);
        VDP_setReg(VDP_MODE_REG_3, VDP_getReg(VDP_MODE_REG_3) | VDP_IE2);
        SYS_setExtIntCallback(&ext_int_callback);
    }
}

void serial_send(u8 data)
{
    mem_write_u8(PORT2_TX, data);
}

bool serial_readyToSend(void)
{
    return !(mem_read_u8(PORT2_SCTRL) & SCTRL_TFUL);
}

void serial_sendWhenReady(u8 data)
{
    while (!serial_readyToSend())
        ;
    serial_send(data);
}
