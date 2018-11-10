#include "ssf.h"
#include "types.h"

#define SSF_REG16(reg) *((volatile u16*)(0xA13000 + reg))

#define REG_USB 226 //usb io
#define REG_STE 228 //status
#define STE_USB_RD_RDY 4 //usb read ready bit

u16 ssf_usb_rd_ready()
{
    return SSF_REG16(REG_STE) & STE_USB_RD_RDY;
}

u8 ssf_usb_read()
{
    return SSF_REG16(REG_USB);
}
