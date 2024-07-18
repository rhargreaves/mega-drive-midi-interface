#include "comm_everdrive_pro.h"
#include "everdrive_led.h"

#define CMD_USB_WR 0x22

#define REG_FIFO_DATA *((vu16*)0xA130D0) // fifo data register
#define REG_FIFO_STAT *((vu16*)0xA130D2) // fifo status register. shows if fifo can be readed.
#define REG_SYS_STAT *((vu16*)0xA130D4)

#define FIFO_CPU_RXF 0x8000 // fifo flags. system cpu can read
#define FIFO_RXF_MSK 0x7FF
#define STAT_PRO_PRESENT 0xA0

static bool pro_present(void)
{
    return REG_SYS_STAT & STAT_PRO_PRESENT;
}

static u8 bi_fifo_busy(void)
{
    return (REG_FIFO_STAT & FIFO_CPU_RXF) ? 1 : 0;
}

static void bi_fifo_rd(void* data, u16 len)
{
    u8* data8 = data;
    u16 block = 0;

    while (len) {

        block = REG_FIFO_STAT & FIFO_RXF_MSK;
        if (block > len)
            block = len;
        len -= block;

        while (block >= 4) {
            *data8++ = REG_FIFO_DATA;
            *data8++ = REG_FIFO_DATA;
            *data8++ = REG_FIFO_DATA;
            *data8++ = REG_FIFO_DATA;
            block -= 4;
        }

        while (block--)
            *data8++ = REG_FIFO_DATA;
    }
}

static void bi_fifo_wr(void* data, u16 len)
{
    u8* data8 = data;

    while (len--) {
        REG_FIFO_DATA = *data8++;
    }
}

static void bi_cmd_tx(u8 cmd)
{
    u8 buff[4];
    buff[0] = '+';
    buff[1] = '+' ^ 0xff;
    buff[2] = cmd;
    buff[3] = cmd ^ 0xff;
    bi_fifo_wr(buff, sizeof(buff));
}

static void bi_cmd_usb_wr(void* data, u16 len)
{
    bi_cmd_tx(CMD_USB_WR);
    bi_fifo_wr(&len, 2);
    bi_fifo_wr(data, len);
}

u8 comm_everdrive_pro_read_ready(void)
{
    if (!pro_present()) {
        return 0;
    }

    return !bi_fifo_busy();
}

u8 comm_everdrive_pro_read(void)
{
    everdrive_led_blink();

    u8 data;
    bi_fifo_rd(&data, 1);
    return data;
}

u8 comm_everdrive_pro_write_ready(void)
{
    return TRUE;
}

void comm_everdrive_pro_write(u8 data)
{
    bi_cmd_usb_wr(&data, 1);
    (void)data;
}

void comm_everdrive_pro_init(void)
{
}
