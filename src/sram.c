#include "sram.h"
#include "sram_ed_pro.h"
#include "sram_ed_x7.h"
#include "comm/comm_everdrive_pro.h"
#include "comm/comm_everdrive.h"

typedef struct SramVTable {
    void (*enable)(bool write);
    void (*disable)(void);
    void (*write)(u32 offset, u8 value);
    u8 (*read)(u32 offset);
} SramVTable;

static const SramVTable sram_vtable_none = {
    .enable = NULL,
    .disable = NULL,
    .write = NULL,
    .read = NULL,
};

static const SramVTable sram_vtable_ed_pro = {
    .enable = sram_ed_pro_enable,
    .disable = sram_ed_pro_disable,
    .write = sram_ed_pro_write,
    .read = sram_ed_pro_read,
};

static const SramVTable sram_vtable_ed_x7 = {
    .enable = sram_ed_x7_enable,
    .disable = sram_ed_x7_disable,
    .write = sram_ed_x7_write,
    .read = sram_ed_x7_read,
};

static const SramVTable* sram_ops;
static bool sram_z80_bus_taken;

void sram_init(void)
{
    if (comm_everdrive_pro_is_present()) {
        sram_ops = &sram_vtable_ed_pro;
    } else if (comm_everdrive_is_present()) {
        sram_ops = &sram_vtable_ed_x7;
    } else {
        sram_ops = &sram_vtable_none;
    }
    sram_z80_bus_taken = false;
}

void sram_enable(bool write)
{
    SYS_disableInts();
    sram_z80_bus_taken = Z80_getAndRequestBus(TRUE);
    sram_ops->enable(write);
}

void sram_disable(void)
{
    sram_ops->disable();
    if (sram_z80_bus_taken) {
        Z80_releaseBus();
        sram_z80_bus_taken = false;
    }
    SYS_enableInts();
}

void sram_write(u32 offset, u8 value)
{
    sram_ops->write(offset, value);
}

u8 sram_read(u32 offset)
{
    return sram_ops->read(offset);
}

bool sram_is_present(void)
{
    return sram_ops != &sram_vtable_none;
}