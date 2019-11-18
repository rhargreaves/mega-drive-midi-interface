#include "comm.h"
#include "comm_everdrive.h"
#include "comm_serial.h"
#include <stdbool.h>

static u16 idle = 0;
static u16 reads = 0;

static const u16 MAX_COMM_IDLE = 0x28F;
static const u16 MAX_COMM_BUSY = 0x28F;

static const u16 COMM_TYPES = 2;

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

static const CommVTable* commTypes[] = { &Everdrive_VTable, &Serial_VTable };

static const CommVTable* activeCommType = NULL;

void comm_init(void)
{
    for (u16 i = 0; i < COMM_TYPES; i++) {
        commTypes[i]->init();
    }
    activeCommType = NULL;
}

static bool readReady(void)
{
    if (activeCommType == NULL) {
        for (u16 i = 0; i < COMM_TYPES; i++) {
            if (commTypes[i]->readReady()) {
                activeCommType = commTypes[i];
                return true;
            }
        }
        return false;
    } else if (activeCommType->readReady()) {
        return true;
    } else {
        if (countsInBounds()) {
            idle++;
        }
        return false;
    }
}

bool comm_readReady(void)
{
    return readReady();
}

u8 comm_read(void)
{
    while (!readReady())
        ;
    if (countsInBounds()) {
        reads++;
    }
    return activeCommType->read();
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
    while (!activeCommType->writeReady())
        ;
    activeCommType->write(data);
}

CommMode comm_mode(void)
{
    if (activeCommType == &Everdrive_VTable) {
        return Everdrive;
    } else if (activeCommType == &Serial_VTable) {
        return Serial;
    } else {
        return Discovery;
    }
}

static bool countsInBounds(void)
{
    return idle != MAX_COMM_IDLE && reads != MAX_COMM_BUSY;
}
