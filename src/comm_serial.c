#include "comm_serial.h"
#include "buffer.h"
#include "serial.h"

u16 comm_serial_baudRate(void)
{
    switch (serial_sctrl() & 0xC0) {
    case SCTRL_300_BPS:
        return 300;
    case SCTRL_1200_BPS:
        return 1200;
    case SCTRL_2400_BPS:
        return 2400;
    default:
        return 4800;
    }
}

static void updateBuffer(void)
{
    while (serial_readyToReceive()) {
        buffer_write(serial_receive());
    }
}

static void recvReadyCallback(void)
{
    updateBuffer();
}

void comm_serial_init(void)
{
    serial_init(SCTRL_4800_BPS | SCTRL_SIN | SCTRL_SOUT | SCTRL_RINT);
    serial_setReadyToReceiveCallback(&recvReadyCallback);
}

u8 comm_serial_readReady(void)
{
    return buffer_canRead();
}

u8 comm_serial_read(void)
{
    return buffer_read();
}

u8 comm_serial_writeReady(void)
{
    return serial_readyToSend();
}

void comm_serial_write(u8 data)
{
    serial_send(data);
}
