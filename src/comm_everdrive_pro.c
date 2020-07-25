#include "comm_everdrive_pro.h"
#include <stdbool.h>

#define REG_FIFO_DATA *((vu16*)0xA130D0) // fifo data register
#define REG_FIFO_STAT                                                          \
    *((vu16*)0xA130D2) // fifo status register. shows if fifo can be readed.
#define REG_SYS_STAT *((vu16*)0xA130D4)

#define FIFO_CPU_RXF 0x8000 // fifo flags. system cpu can read
#define FIFO_RXF_MSK 0x7FF
#define STAT_PRO_PRESENT 0xA0

#define BUFFER_SIZE 1

static u8 buffer[BUFFER_SIZE];

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

        while (block--) *data8++ = REG_FIFO_DATA;
    }
}

u8 comm_everdrive_pro_readReady(void)
{
    if (!pro_present()) {
        return 0;
    }

    return !bi_fifo_busy();
}

u8 comm_everdrive_pro_read(void)
{
    bi_fifo_rd(buffer, 1);
    return buffer[0];
}

u8 comm_everdrive_pro_writeReady(void)
{
    return 0;
}

void comm_everdrive_pro_write(u8 data)
{
    (void)data;
}

void comm_everdrive_pro_init(void)
{
    buffer[0] = 0;
}
