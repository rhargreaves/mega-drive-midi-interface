#include "comm_megawifi.h"
#include "applemidi.h"
#include "log.h"
#include "mw/loop.h"
#include "mw/megawifi.h"
#include "mw/mpool.h"
#include "mw/util.h"
#include "vstring.h"
#include <stdbool.h>
#include "settings.h"
#include "buffer.h"

/// Length of the wflash buffer
#define MW_BUFLEN 1460

/// Command buffer
static char cmd_buf[MW_BUFLEN];
static bool mw_detected = false;
static bool recvData = false;

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
    log_info("MegaWiFi: Connecting AP");
    mw_err err = mw_ap_assoc(0);
    if (err != MW_ERR_NONE) {
        return err;
    }
    err = mw_ap_assoc_wait(MS_TO_FRAMES(20000));
    if (err != MW_ERR_NONE) {
        return err;
    }
    log_info("MegaWiFi: Connected.");
    return MW_ERR_NONE;
}

static mw_err display_ip_addr(void)
{
    struct mw_ip_cfg* ip_cfg;
    mw_err err = mw_ip_current(&ip_cfg);
    if (err != MW_ERR_NONE) {
        return err;
    }
    char ip_str[16] = {};
    uint32_to_ip_str(ip_cfg->addr.addr, ip_str);
    log_info("MegaWiFi: IP: %s", ip_str);
    return err;
}

bool detect_mw(void)
{
    u8 ver_major = 0, ver_minor = 0;
    char* variant = NULL;
    mw_err err = mw_detect(&ver_major, &ver_minor, &variant);
    if (MW_ERR_NONE != err) {
        if (settings_isMegaWiFiRom()) {
            log_warn("MegaWiFi: Not found");
        }
        return false;
    }
    log_info("MegaWiFi: Found v%d.%d", ver_major, ver_minor);
    return true;
}

static void open_udp_port(u8 ch, u16 dst_port, u16 src_port, const char* name)
{
    char dst_port_str[6];
    v_sprintf(dst_port_str, "%d", dst_port);
    char src_port_str[6];
    v_sprintf(src_port_str, "%d", src_port);
    mw_err err = mw_udp_set(ch, "127.0.0.1", dst_port_str, src_port_str);
    if (err != MW_ERR_NONE) {
        return;
    }
    err = mw_sock_conn_wait(ch, MS_TO_FRAMES(1000));
    if (err != MW_ERR_NONE) {
        return;
    }
    log_info("AppleMIDI: %s UDP Port: %d", name, dst_port);
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
    display_ip_addr();
    open_udp_port(CH_CONTROL_PORT, 5004, 5006, "Control");
    open_udp_port(CH_MIDI_PORT, 5005, 5007, "MIDI");
}

u8 comm_megawifi_readReady(void)
{
    if (!recvData)
        return false;
    return buffer_canRead();
}

u8 comm_megawifi_read(void)
{
    return buffer_read();
}

u8 comm_megawifi_writeReady(void)
{
    return 0;
}

void comm_megawifi_write(u8 data)
{
    (void)data;
}

#define MAX_UDP_DATA_LENGTH 1460

static char recvBuffer[MAX_UDP_DATA_LENGTH];

static bool awaitingRecv = false;
static bool awaitingSend = false;

static void process_udp_data(u8 ch, char* buffer, u16 length)
{
    UNUSED_PARAM(buffer);
    mw_err err = MW_ERR_NONE;
    switch (ch) {
    case CH_CONTROL_PORT:
        err = applemidi_processSessionControlPacket(buffer, length);
        break;
    case CH_MIDI_PORT:
        err = applemidi_processSessionMidiPacket(buffer, length);
        break;
    }
    if (err != MW_ERR_NONE) {
        log_warn("MegaWiFi: process_udp_data() = %d", err);
    }
}

static void recv_complete_cb(
    enum lsd_status stat, uint8_t ch, char* data, uint16_t len, void* ctx)
{
    UNUSED_PARAM(ctx);

    if (LSD_STAT_COMPLETE == stat) {
        process_udp_data(ch, data, len);
    } else {
        log_warn("MegaWiFi: recv_complete_cb() = %d", stat);
    }
    awaitingRecv = false;
}

static u16 lastSeqNum = 0;

void comm_megawifi_tick(void)
{
    if (!mw_detected)
        return;
    mw_process();
    if (awaitingRecv || awaitingSend) {
        return;
    }

    u16 seqNum = applemidi_lastSequenceNumber();
    if (lastSeqNum != seqNum) {
        applemidi_sendReceiverFeedback();
        lastSeqNum = seqNum;
    }

    awaitingRecv = true;
    enum lsd_status stat
        = mw_recv(recvBuffer, MAX_UDP_DATA_LENGTH, NULL, recv_complete_cb);
    if (stat < 0) {
        log_warn("MegaWiFi: mw_recv() = %d", stat);
        awaitingRecv = false;
        return;
    }
}

void comm_megawifi_midiEmitCallback(u8 data)
{
    recvData = true;
    buffer_write(data);
}

#define MAX_UDP_DATA_LENGTH 1460

void send_complete_cb(enum lsd_status stat, void* ctx)
{
    UNUSED_PARAM(ctx);
    if (stat < 0) {
        log_warn("MegaWiFi: send_complete_cb() = %d", stat);
    }
    awaitingSend = false;
}

void comm_megawifi_send(u8 ch, char* data, u16 len)
{
    awaitingSend = true;
    enum lsd_status stat = mw_send(ch, data, len, NULL, send_complete_cb);
    if (stat < 0) {
        log_warn("MegaWiFi: mw_send() = %d", stat);
        awaitingSend = false;
        return;
    }
}
