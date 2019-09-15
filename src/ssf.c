#include "ssf.h"
#include <types.h>

#define SSF_REG16(reg) *((volatile u16*)(0xA13000 + reg))
#define REG_USB 226
#define REG_STE 228
#define STE_USB_RD_RDY 4
#define STE_USB_WR_RDY 2 // usb write ready bit

u16 ssf_usb_rd_ready(void)
{
    return SSF_REG16(REG_STE) & STE_USB_RD_RDY;
}

u8 ssf_usb_read(void)
{
    return SSF_REG16(REG_USB);
}

u16 ssf_usb_wr_ready(void)
{
    return SSF_REG16(REG_STE) & STE_USB_WR_RDY;
}

u8 ssf_usb_write(u8 data)
{
    return SSF_REG16(REG_USB) = data;
}
