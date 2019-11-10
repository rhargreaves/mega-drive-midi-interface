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

typedef struct CommVTable CommVTable;

struct CommVTable {
    void (*init)(void);
    u8 (*readReady)(void);
    u8 (*read)(void);
    u8 (*writeReady)(void);
    void (*write)(u8 data);
};

static const CommVTable Everdrive_VTable
    = { comm_everdrive_init, comm_everdrive_readReady, comm_everdrive_read,
          comm_everdrive_writeReady, comm_everdrive_write };

static const CommVTable Serial_VTable
    = { comm_serial_init, comm_serial_readReady, comm_serial_read,
          comm_serial_writeReady, comm_serial_write };

#if SERIAL
static const CommVTable* ops = &Serial_VTable;
#else
static const CommVTable* ops = &Everdrive_VTable;
#endif

void comm_init(void)
{
    (void)Serial_VTable;
    (void)Everdrive_VTable;
    ops->init();
}

u8 comm_read(void)
{
    waitForReady();
    if (countsInBounds()) {
        reads++;
    }
    return ops->read();
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
    while (!ops->writeReady())
        ;
    ops->write(data);
}

static void waitForReady(void)
{
    while (!ops->readReady()) {
        if (countsInBounds()) {
            idle++;
        }
    }
}

static bool countsInBounds(void)
{
    return idle != MAX_COMM_IDLE && reads != MAX_COMM_BUSY;
}
