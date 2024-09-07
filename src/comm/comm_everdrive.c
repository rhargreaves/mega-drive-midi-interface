#include "comm_everdrive.h"
#include "everdrive_led.h"

#define SSF_REG16(reg) *((volatile u16*)(0xA13000 + reg))
#define REG_USB 0xE2
#define REG_STE 0xE4
#define STE_USB_RD_RDY 4
#define STE_USB_WR_RDY 2 // usb write ready bit
#define STE_SPI_RDY 1

#define IO_STATUS_HI_SD 0x00
#define IO_STATUS_HI_SDHC 0x40

void comm_everdrive_init(void)
{
}

bool comm_everdrive_is_present(void)
{
    /* REG_STE values:
    0x3F00 with OpenEmu v2.4.1
    0x3F00 with Exodus
    0x3F00 with Regen v0.97d
    0xFFFF with Fusion 3.6.4
    0x3F00 with BlastEm nightly (0.6.3-pre-4c418ee9a9d8) Win
    0x3015 with BlastEm nightly (0.6.3-pre) (also 0x3014)
    0x4003 when ME X7 idle with USB in
    0x4009 when ME X7 idle loaded via SD without USB cable connected
    0x4003 when ME X7 idle loaded via SD with USB cable connected
    0x3F00 with ME PRO
    */

    u8 status = SSF_REG16(REG_STE) >> 8;
    return status == IO_STATUS_HI_SD || status == IO_STATUS_HI_SDHC;
}

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
