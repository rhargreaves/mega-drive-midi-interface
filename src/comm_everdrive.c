#include "comm_everdrive.h"
#include "everdrive_led.h"

#define SSF_REG16(reg) *((volatile u16*)(0xA13000 + reg))
#define REG_USB 226
#define REG_STE 228
#define STE_USB_RD_RDY 4
#define STE_USB_WR_RDY 2 // usb write ready bit

u8 comm_everdrive_read_ready(void)
{
    return SSF_REG16(REG_STE) & STE_USB_RD_RDY;
}

u8 comm_everdrive_read(void)
{
    everdrive_led_blink();
    return SSF_REG16(REG_USB);
}

u8 comm_everdrive_write_ready(void)
{
    return SSF_REG16(REG_STE) & STE_USB_WR_RDY;
}

void comm_everdrive_write(u8 data)
{
    SSF_REG16(REG_USB) = data;
}

void comm_everdrive_init(void)
{
}
