#include "comm.h"
#include "comm_everdrive.h"
#include "comm_everdrive_pro.h"
#include "comm_megawifi.h"
#include "comm_serial.h"
#include "comm_demo.h"
#include "settings.h"

static u16 idle = 0;
static u16 reads = 0;

static const u16 MAX_COMM_IDLE = 0x28F;
static const u16 MAX_COMM_BUSY = 0x28F;

static bool counts_in_bounds(void);

typedef struct CommVTable {
    void (*init)(void);
    bool (*is_present)(void);
    u8 (*read_ready)(void);
    u8 (*read)(void);
    u8 (*write_ready)(void);
    void (*write)(u8 data);
} CommVTable;

static const CommVTable Demo_VTable = { comm_demo_init, comm_demo_is_present, comm_demo_read_ready,
    comm_demo_read, comm_demo_write_ready, comm_demo_write };

static const CommVTable Everdrive_VTable
    = { comm_everdrive_init, comm_everdrive_is_present, comm_everdrive_read_ready,
          comm_everdrive_read, comm_everdrive_write_ready, comm_everdrive_write };

static const CommVTable EverdrivePro_VTable
    = { comm_everdrive_pro_init, comm_everdrive_pro_is_present, comm_everdrive_pro_read_ready,
          comm_everdrive_pro_read, comm_everdrive_pro_write_ready, comm_everdrive_pro_write };

static const CommVTable Serial_VTable = { comm_serial_init, comm_serial_is_present,
    comm_serial_read_ready, comm_serial_read, comm_serial_write_ready, comm_serial_write };

static const CommVTable Megawifi_VTable = { comm_megawifi_init, comm_megawifi_is_present,
    comm_megawifi_read_ready, comm_megawifi_read, comm_megawifi_write_ready, comm_megawifi_write };

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
#if MEGAWIFI && COMM_MEGAWIFI == 1
    &Megawifi_VTable,
#endif
    &Demo_VTable
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

static bool read_ready(void)
{
    if (activeCommType == NULL) {
        for (u16 i = 0; i < COMM_TYPES; i++) {
            if (commTypes[i]->is_present() && commTypes[i]->read_ready()) {
                activeCommType = commTypes[i];
                return true;
            }
        }
        return false;
    } else if (activeCommType->read_ready()) {
        return true;
    } else {
        if (counts_in_bounds()) {
            idle++;
        }
        return false;
    }
}

bool comm_read_ready(void)
{
    return read_ready();
}

u8 comm_read(void)
{
    while (!read_ready())
        ;
    if (counts_in_bounds()) {
        reads++;
    }
    return activeCommType->read();
}

u16 comm_idle_count(void)
{
    return idle;
}

u16 comm_busy_count(void)
{
    return reads;
}

void comm_reset_counts(void)
{
    idle = 0;
    reads = 0;
}

void comm_write(u8 data)
{
    while (!activeCommType->write_ready())
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
    } else if (activeCommType == &Demo_VTable) {
        return Demo;
    } else {
        return Discovery;
    }
}

static bool counts_in_bounds(void)
{
    return idle != MAX_COMM_IDLE && reads != MAX_COMM_BUSY;
}
