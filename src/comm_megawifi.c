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
#include "memory.h"

/// Length of the wflash buffer
#define MW_BUFLEN 1460

/// Command buffer
static char cmd_buf[MW_BUFLEN];
static bool mw_detected = false;
static bool recvData = false;

#define MAX_UDP_DATA_LENGTH 1460

static char recvBuffer[MAX_UDP_DATA_LENGTH];
static char sendBuffer[MAX_UDP_DATA_LENGTH];

#define REUSE_PAYLOAD_HEADER_LEN 6

static bool awaitingRecv = false;
static bool awaitingSend = false;

#define MW_MAX_LOOP_FUNCS 2
#define MW_MAX_LOOP_TIMERS 4

static void recv_complete_cb(
    enum lsd_status stat, uint8_t ch, char* data, uint16_t len, void* ctx);

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

static mw_err associateAp(void)
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

static mw_err displayLocalIp(void)
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

static void listenOnUdpPort(u8 ch, u16 src_port, const char* name)
{
    struct mw_reuse_payload* pkt = (struct mw_reuse_payload* const)recvBuffer;

    char src_port_str[6];
    v_sprintf(src_port_str, "%d", src_port);
    mw_err err = mw_udp_set(ch, NULL, NULL, src_port_str);
    if (err != MW_ERR_NONE) {
        return;
    }
    enum lsd_status stat
        = mw_udp_reuse_recv(pkt, MW_BUFLEN, NULL, recv_complete_cb);
    if (stat < 0) {
        log_warn("MW: mw_udp_reuse_recv() = %d", stat);
        return;
    }
    log_info("AppleMIDI: %s UDP Port: %d", name, src_port);
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
    associateAp();
    displayLocalIp();
    listenOnUdpPort(CH_CONTROL_PORT, 5006, "Control");
    listenOnUdpPort(CH_MIDI_PORT, 5007, "MIDI");
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

static void processUdpData(u8 ch, char* buffer, u16 length)
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
        log_warn("MegaWiFi: processUdpData() = %d", err);
    }
}

static u32 remoteIp = 0;
static u16 remoteControlPort = 0;
static u16 remoteMidiPort = 0;

static void recv_complete_cb(
    enum lsd_status stat, uint8_t ch, char* data, uint16_t len, void* ctx)
{
    UNUSED_PARAM(ctx);
    struct mw_reuse_payload* udp = (struct mw_reuse_payload*)data;

    if (LSD_STAT_COMPLETE == stat) {
        remoteIp = udp->remote_ip;
        if (ch == CH_CONTROL_PORT) {
            remoteControlPort = udp->remote_port;
        } else if (ch == CH_MIDI_PORT) {
            remoteMidiPort = udp->remote_port;
        }
        processUdpData(ch, udp->payload, len);
    } else {
        log_warn("MegaWiFi: recv_complete_cb() = %d", stat);
    }

    enum lsd_status r_stat
        = mw_udp_reuse_recv(udp, MW_BUFLEN, NULL, recv_complete_cb);
    if (r_stat < 0) {
        log_warn("MegaWiFi: mw_udp_reuse_recv() = %d", r_stat);
    }
    awaitingRecv = false;
}

static u16 frame = 0;

void comm_megawifi_vsync(void)
{
    frame++;
}

static u16 lastSeqNum = 0;

#define RECEIVER_FEEDBACK_FRAME_FREQUENCY 25

static void sendReceiverFeedback(void)
{
    if (frame < RECEIVER_FEEDBACK_FRAME_FREQUENCY) {
        return;
    }

    u16 seqNum = applemidi_lastSequenceNumber();
    if (lastSeqNum != seqNum) {
        applemidi_sendReceiverFeedback();
        lastSeqNum = seqNum;
    }
    frame = 0;
}

void comm_megawifi_tick(void)
{
    if (!mw_detected)
        return;
    mw_process();
    if (awaitingRecv || awaitingSend) {
        return;
    }
    sendReceiverFeedback();
}

void comm_megawifi_midiEmitCallback(u8 data)
{
    recvData = true;
    if (!buffer_canWrite()) {
        log_warn("MegaWiFi: MIDI buffer full!");
        return;
    }
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
    struct mw_reuse_payload* udp = (struct mw_reuse_payload*)sendBuffer;
    udp->remote_ip = remoteIp;
    udp->remote_port
        = ch == CH_CONTROL_PORT ? remoteControlPort : remoteMidiPort;
    memcpy(udp->payload, data, len);

    // char ip_buf[16];
    // uint32_to_ip_str(remoteIp, ip_buf);
    // log_info("MW: Send IP=%s:%d L=%d C=%d", ip_buf, udp->remote_port, len, ch);

    awaitingSend = true;
    enum lsd_status stat = mw_udp_reuse_send(
        ch, udp, len + REUSE_PAYLOAD_HEADER_LEN, NULL, send_complete_cb);
    if (stat < 0) {
        log_warn("MegaWiFi: mw_udp_reuse_send() = %d", stat);
        awaitingSend = false;
        return;
    }
}
