#include <comm.h>
#include <ssf.h>

static u16 idleTime = 0;

u8 comm_read(void)
{
    while (!ssf_usb_rd_ready()) {
        idleTime++;
        if (idleTime > 10000) {
            idleTime = 10000;
        }
    }
    idleTime = 0;
    return ssf_usb_read();
}

u16 comm_idle(void)
{
    return idleTime;
}
