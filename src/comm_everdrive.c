#include "comm_everdrive.h"
#include "ssf.h"

u8 comm_everdrive_readReady(void)
{
    return ssf_usb_rd_ready();
}

u8 comm_everdrive_read(void)
{
    return ssf_usb_read();
}

u8 comm_everdrive_writeReady(void)
{
    return ssf_usb_wr_ready();
}

void comm_everdrive_write(u8 data)
{
    ssf_usb_write(data);
}
