#include <comm.h>
#include <ssf.h>

u8 comm_read(void)
{
    while (!ssf_usb_rd_ready())
        ;
    return ssf_usb_read();
}
