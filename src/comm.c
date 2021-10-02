#include "comm.h"
#include "comm_everdrive.h"
#include "comm_everdrive_pro.h"
#include "comm_megawifi.h"
#include "comm_serial.h"
#include <stdbool.h>
#include <vdp.h>
#include <vdp_bg.h>
#include "vstring.h"
#include "settings.h"

static u16 idle = 0;
static u16 reads = 0;

static const u16 MAX_COMM_IDLE = 0x28F;
static const u16 MAX_COMM_BUSY = 0x28F;

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
    = { comm_everdrive_init, comm_everdrive_read_ready, comm_everdrive_read,
          comm_everdrive_write_ready, comm_everdrive_write };

static const CommVTable EverdrivePro_VTable = { comm_everdrive_pro_init,
    comm_everdrive_pro_read_ready, comm_everdrive_pro_read,
    comm_everdrive_pro_write_ready, comm_everdrive_pro_write };

static const CommVTable Serial_VTable
    = { comm_serial_init, comm_serial_read_ready, comm_serial_read,
          comm_serial_write_ready, comm_serial_write };

static const CommVTable Megawifi_VTable
    = { comm_megawifi_init, comm_megawifi_read_ready, comm_megawifi_read,
          comm_megawifi_write_ready, comm_megawifi_write };

static const CommVTable* commTypes[] = {
#if COMM_EVERDRIVE_X7 == 1
    &Everdrive_VTable,
#endif
#if COMM_EVERDRIVE_PRO == 1
    &EverdrivePro_VTable,
#endif
#if COMM_SERIAL == 1
    &Serial_VTable,
#endif
#if COMM_MEGAWIFI == 1
    &Megawifi_VTable
#endif
};

#define COMM_TYPES (sizeof(commTypes) / sizeof(CommVTable*))

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
    } else if (activeCommType == &EverdrivePro_VTable) {
        return EverdrivePro;
    } else if (activeCommType == &Serial_VTable) {
        return Serial;
    } else if (activeCommType == &Megawifi_VTable) {
        return MegaWiFi;
    } else {
        return Discovery;
    }
}

static bool countsInBounds(void)
{
    return idle != MAX_COMM_IDLE && reads != MAX_COMM_BUSY;
}
