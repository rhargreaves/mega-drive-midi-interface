#include "serial.h"
#include "mem.h"

#define VDP_MODE_REG_3 0xB
#define VDP_IE2 0x08
#define INT_MASK_LEVEL_ENABLE_ALL 1

static VoidCallback* readReadyCallback;
static IoPort io_port;

static const struct {
    size_t ctrl;
    size_t sctrl;
    size_t tx;
    size_t rx;
} regs[] = {
    { PORT2_CTRL, PORT2_SCTRL, PORT2_TX, PORT2_RX },
    { EXT_CTRL, EXT_SCTRL, EXT_TX, EXT_RX },
};

static void ext_int_callback(void)
{
    readReadyCallback();
}

static void set_sctrl(u8 value)
{
    mem_write_u8(regs[io_port].sctrl, value);
}

static void set_ctrl(u8 value)
{
    mem_write_u8(regs[io_port].ctrl, value);
}

u8 serial_sctrl(void)
{
    return mem_read_u8(regs[io_port].sctrl);
}

bool serial_readyToReceive(void)
{
    return mem_read_u8(regs[io_port].sctrl) & SCTRL_RRDY;
}

u8 serial_receive(void)
{
    return mem_read_u8(regs[io_port].rx);
}

void serial_setReadyToReceiveCallback(VoidCallback* cb)
{
    readReadyCallback = cb;
}

void serial_init(IoPort port, u8 sctrlFlags)
{
    io_port = port;
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
    mem_write_u8(regs[io_port].tx, data);
}

bool serial_readyToSend(void)
{
    return !(mem_read_u8(regs[io_port].sctrl) & SCTRL_TFUL);
}

void serial_sendWhenReady(u8 data)
{
    while (!serial_readyToSend())
        ;
    serial_send(data);
}
