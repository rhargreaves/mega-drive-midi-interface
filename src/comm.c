#include "comm.h"
#include "comm_everdrive.h"
#include "comm_serial.h"
#include <stdbool.h>

static u16 idle = 0;
static u16 reads = 0;

static const u16 MAX_COMM_IDLE = 0x28F;
static const u16 MAX_COMM_BUSY = 0x28F;

static void waitForReady(void);
static bool countsInBounds(void);

void comm_init(void)
{
}

u8 comm_read(void)
{
    waitForReady();
    if (countsInBounds()) {
        reads++;
    }
    return comm_everdrive_read();
}

u16 comm_idleCount(void)
{
    return idle;
}

u16 comm_busyCount(void)
{
    return reads;
}

void comm_resetCounts(void)
{
    idle = 0;
    reads = 0;
}

void comm_write(u8 data)
{
    while (!comm_everdrive_writeReady())
        ;
    comm_everdrive_write(data);
}

static void waitForReady(void)
{
    while (!comm_everdrive_readReady()) {
        if (countsInBounds()) {
            idle++;
        }
    }
}

static bool countsInBounds(void)
{
    return idle != MAX_COMM_IDLE && reads != MAX_COMM_BUSY;
}
