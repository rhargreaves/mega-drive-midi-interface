#include "ssf.h"
#include "types.h"

#define SSF_REG16(reg) *((volatile u16*)(0xA13000 + reg))
#define SSF_REG32(reg) *((volatile u32*)(0xA13000 + reg))

#define REG_MATH 208
#define REG_SPI 224 //SD io
#define REG_USB 226 //usb io
#define REG_STE 228 //status
#define REG_CFG 230 //IO config
#define REG_SSF_CTRL 240

#define SSF_CTRL_P 0x8000 //register accesss protection bit. should be set, otherwise register will ignore any attempts to write
#define SSF_CTRL_X 0x4000 //32x mode
#define SSF_CTRL_W 0x2000 //ROM memory write protection
#define SSF_CTRL_L 0x1000 //led

#define STE_SPI_RDY 1 //spi ready bit. SPI should work faster than m68k cpu can read, so, probably this is useless bit
#define STE_USB_WR_RDY 2 //usb write ready bit
#define STE_USB_RD_RDY 4 //usb read ready bit
#define STE_CARD 16384 //SD card type. 0=SD, 1=SDHC

//spi chip select signal
#define CFG_SPI_SS 1
#define CFG_SPI_QRD 6
#define CFG_SPI_QWR 2

volatile u16 ctrl;
volatile u16 cfg_io;

void ssf_init()
{
    u16 i;
    ctrl = SSF_CTRL_P;
    cfg_io = CFG_SPI_SS;

    SSF_REG16(REG_SSF_CTRL) = ctrl;
    SSF_REG16(REG_CFG) = cfg_io;

    for (i = 1; i < 8; i++) ssf_set_rom_bank(i, i);
}

void ssf_set_rom_bank(u8 bank, u8 val)
{
    SSF_REG16(REG_SSF_CTRL + (u32)bank * 2) = val;
}

void ssf_led_off()
{
    ctrl &= ~SSF_CTRL_L;
    SSF_REG16(REG_SSF_CTRL) = ctrl;
}

void ssf_led_on()
{
    ctrl |= SSF_CTRL_L;
    SSF_REG16(REG_SSF_CTRL) = ctrl;
}

u16 ssf_usb_rd_ready()
{
    return SSF_REG16(REG_STE) & STE_USB_RD_RDY;
}

u16 ssf_usb_wr_ready()
{
    return SSF_REG16(REG_STE) & STE_USB_WR_RDY;
}

u16 ssd_spi_ready()
{
    return SSF_REG16(REG_STE) & STE_SPI_RDY;
}

u8 ssf_usb_read()
{
    return SSF_REG16(REG_USB);
}

u8 ssf_usb_write(u8 data)
{
    return SSF_REG16(REG_USB) = data;
}

u8 ssf_spi(u8 data)
{
    SSF_REG16(REG_SPI) = data;
    return SSF_REG16(REG_SPI);
}

void ssf_spi_ss_off()
{
    cfg_io |= CFG_SPI_SS;
    SSF_REG16(REG_CFG) = cfg_io;
}

void ssf_spi_ss_on()
{
    cfg_io &= ~CFG_SPI_SS;
    SSF_REG16(REG_CFG) = cfg_io;
}

void ssf_spi_qrd_on()
{
    cfg_io |= CFG_SPI_QRD;
    SSF_REG16(REG_CFG) = cfg_io;
}

void ssf_spi_qrd_off()
{
    cfg_io &= ~CFG_SPI_QRD;
    SSF_REG16(REG_CFG) = cfg_io;
}

//QRD is for fast reading of SPI.
//1)Allow to read 16bit words instead of 8bit.
//2)Eliminate needs to write before than read

u16 ssf_spi_qrd()
{
    return SSF_REG16(REG_SPI);
}

void ssf_spi_qwr_on()
{
    cfg_io |= CFG_SPI_QWR;
    SSF_REG16(REG_CFG) = cfg_io;
}

void ssf_spi_qwr_off()
{
    cfg_io &= ~CFG_SPI_QWR;
    SSF_REG16(REG_CFG) = cfg_io;
}

//QWR if for 16 bit write to SD card

void ssf_spi_qwr(u16 data)
{
    SSF_REG16(REG_SPI) = data;
}

u16 ssf_card_type()
{
    return SSF_REG16(REG_STE) & STE_CARD;
}

void ssf_rom_wr_on()
{
    ctrl |= SSF_CTRL_W;
    SSF_REG16(REG_SSF_CTRL) = ctrl;
}

void ssf_rom_wr_off()
{
    ctrl &= ~SSF_CTRL_W;
    SSF_REG16(REG_SSF_CTRL) = ctrl;
}

u32 ssf_mul32(u32 arg1, u32 arg2)
{
    SSF_REG32(REG_MATH + 0) = arg1;
    SSF_REG32(REG_MATH + 4) = arg2;

    return SSF_REG32(REG_MATH);
}

//result = arg1 / arg2

u32 ssf_div32(u32 arg1, u32 arg2)
{
    SSF_REG32(REG_MATH + 0) = arg1;
    SSF_REG32(REG_MATH + 4) = arg2;

    return SSF_REG32(REG_MATH + 4);
}

u16 ssf_mul16(u16 arg1, u16 arg2)
{
    SSF_REG16(REG_MATH + 2) = arg1;
    SSF_REG16(REG_MATH + 6) = arg2;

    return SSF_REG16(REG_MATH + 2);
}

u16 ssf_div16(u16 arg1, u16 arg2)
{
    SSF_REG16(REG_MATH + 2) = arg1;
    SSF_REG16(REG_MATH + 6) = arg2;

    return SSF_REG16(REG_MATH + 6);
}
