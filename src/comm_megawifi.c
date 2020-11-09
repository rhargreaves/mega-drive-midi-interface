#include "comm_megawifi.h"
#include "mw/loop.h"
#include "mw/megawifi.h"
#include "mw/mpool.h"
#include "mw/util.h"

/// Length of the wflash buffer
#define MW_BUFLEN 1460

/// Command buffer
static char cmd_buf[MW_BUFLEN];

void midi_emit(u8 data)
{
    (void)data;
}

/// MegaWiFi initialization
// static void megawifi_init_cb(struct loop_timer* t)
// {
//     UNUSED_PARAM(t);
//     //  uint8_t ver_major = 0, ver_minor = 0;
//     //char* variant = NULL;
//     //enum mw_err err;
//     // char line[] = "MegaWiFi version X.Y";

//     // Try detecting the module
//     // err = mw_detect(&ver_major, &ver_minor, &variant);
//     // if (MW_ERR_NONE != err) {
//     //     // Megawifi not found
//     // } else {
//     //     // Megawifi found

//     //     // Configuration complete, run test function next frame
//     //     // t->timer_cb = udp_test;
//     //     // loop_timer_start(t, 1);
//     // }
// }

void comm_megawifi_init(void)
{
    mp_init(0);
    mw_init(cmd_buf, MW_BUFLEN);
    mw_process();
}

u8 comm_megawifi_readReady(void)
{
    return 0;
}

u8 comm_megawifi_read(void)
{
    return 0;
}

u8 comm_megawifi_writeReady(void)
{
    return 0;
}

void comm_megawifi_write(u8 data)
{
    (void)data;
}
