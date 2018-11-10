#include <comm.h>
#include <ssf.h>

static u16 idle = 0;
static u16 reads = 0;

static void comm_waitForReady(void);
static void comm_clampCounts(void);

u8 comm_read(void)
{
    comm_waitForReady();
    reads++;
    comm_clampCounts();
    return ssf_usb_read();
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

static void comm_waitForReady(void)
{
    while (!ssf_usb_rd_ready()) {
        idle++;
    }
}

static void comm_clampCounts(void)
{
    if (idle == 0xFFFF || reads == 0xFFFF) {
        comm_resetCounts();
    }
}
