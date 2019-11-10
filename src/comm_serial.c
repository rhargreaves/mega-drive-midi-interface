#include "comm_serial.h"
#include "buffer.h"
#include "serial.h"

static u16 baudRate(u8 sctrl)
{
    switch (sctrl & 0xC0) {
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

static void printBaudRate(void)
{
    char baudRateText[9];
    sprintf(baudRateText, "%d bps", baudRate(serial_sctrl()));
    VDP_drawText(baudRateText, 0, 18);
}

static void recvReadyCallback(void)
{
    while (serial_readyToReceive()) {
        buffer_write(serial_receive());
    }
}

void comm_serial_init(void)
{
    serial_init(SCTRL_4800_BPS | SCTRL_SIN | SCTRL_SOUT | SCTRL_RINT);
    serial_setReadyToReceiveCallback(&recvReadyCallback);
    printBaudRate();
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
