#include "comm_serial.h"
#include "ring_buf.h"
#include "serial.h"
#include "log.h"
#include "settings.h"

static bool recvData = false;

u16 baud_rate(void)
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

static void update_buffer(void)
{
    while (serial_readyToReceive()) {
        recvData = true;

        ring_buf_status_t status = ring_buf_write(serial_receive());
        if (status == RING_BUF_FULL) {
            log_warn("Serial: Buffer overflow!");
            break;
        }
    }
}

static void recv_ready_callback(void)
{
    update_buffer();
}

static void flush_rrdy(void)
{
    while (serial_readyToReceive()) {
        serial_receive();
    }
}

void comm_serial_init(void)
{
    ring_buf_init();
    serial_init(SCTRL_4800_BPS | SCTRL_SIN | SCTRL_SOUT | SCTRL_RINT);
    serial_setReadyToReceiveCallback(&recv_ready_callback);
    flush_rrdy();
    if (settings_debug_serial()) {
        log_info("Serial: Baud = %i", baud_rate());
    }
}

bool comm_serial_is_present(void)
{
    return true;
}

u8 comm_serial_read_ready(void)
{
    if (!recvData)
        return false;
    return ring_buf_can_read();
}

u8 comm_serial_read(void)
{
    u8 data = 0;
    ring_buf_status_t status = ring_buf_read(&data);
    if (status == RING_BUF_OK) {
        u16 bufferAvailable = ring_buf_available();
        if (bufferAvailable < 32) {
            log_warn("Serial: Buffer free = %d bytes", bufferAvailable);
        }
        return data;
    }

    if (status == RING_BUF_EMPTY) {
        log_warn("Serial: Attempted read from empty buffer");
    }

    return 0;
}

u8 comm_serial_write_ready(void)
{
    return serial_readyToSend();
}

void comm_serial_write(u8 data)
{
    serial_send(data);
}
