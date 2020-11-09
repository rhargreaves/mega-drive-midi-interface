#include "comm_megawifi.h"
#include "log.h"
#include "mw/loop.h"
#include "mw/megawifi.h"
#include "mw/mpool.h"
#include "mw/util.h"

#include <stdbool.h>

/// Length of the wflash buffer
#define MW_BUFLEN 1460

/// Command buffer
static char cmd_buf[MW_BUFLEN];
static bool mw_detected = false;

void midi_emit(u8 data)
{
    (void)data;
}

#define MW_MAX_LOOP_FUNCS 2
#define MW_MAX_LOOP_TIMERS 4

static void mw_process_loop_cb(struct loop_func* f)
{
    UNUSED_PARAM(f);
    mw_process();
}

static void mw_process_loop_init(void)
{
    loop_init(MW_MAX_LOOP_FUNCS, MW_MAX_LOOP_TIMERS);
    static struct loop_func loop_func = { .func_cb = mw_process_loop_cb };
    loop_func_add(&loop_func);
}

static mw_err associate_ap(void)
{
    log_info("Associating to AP...", 0, 0, 0);
    mw_err err = mw_ap_assoc(0);
    if (err != MW_ERR_NONE) {
        return err;
    }
    err = mw_ap_assoc_wait(MS_TO_FRAMES(2000));
    if (err != MW_ERR_NONE) {
        return err;
    }
    log_info("Done!", 0, 0, 0);

    return MW_ERR_NONE;
}

bool detect_mw(void)
{
    u8 ver_major = 0, ver_minor = 0;
    char* variant = NULL;
    mw_err err = mw_detect(&ver_major, &ver_minor, &variant);
    if (MW_ERR_NONE != err) {
        // Megawifi not found
        log_warn("MegaWiFi not found (err %d)", err, 0, 0);
        return false;
    }
    log_info("Found MegaWiFi %d.%d", ver_major, ver_minor, 0);
    return true;
}

void comm_megawifi_init(void)
{
    mp_init(0);
    mw_process_loop_init();
    mw_init(cmd_buf, MW_BUFLEN);
    mw_detected = detect_mw();
    if (!mw_detected) {
        return;
    }
    associate_ap();
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
