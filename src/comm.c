#include <comm.h>
#include <ssf.h>

static u16 idle = 0;
static u16 reads = 0;

static void waitForReady(void);
static void clampCounts(void);

u8 comm_read(void)
{
    waitForReady();
    reads++;
    clampCounts();
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

static void waitForReady(void)
{
    while (!ssf_usb_rd_ready()) {
        idle++;
    }
}

static void clampCounts(void)
{
    if (idle == 0xFFFF || reads == 0xFFFF) {
        comm_resetCounts();
    }
}
