#include "comm_megawifi.h"
#include "applemidi.h"
#include "log.h"
#include <ext/mw/megawifi.h>
#include <ext/mw/lsd.h>
#include "vstring.h"
#include <stdbool.h>
#include "settings.h"
#include "buffer.h"
#include "memory.h"
#include "ip_util.h"
#include <task.h>
#include "scheduler.h"

#define UDP_CONTROL_PORT 5006
#define UDP_MIDI_PORT (UDP_CONTROL_PORT + 1)

#define MW_BUFLEN 1460
#define MAX_UDP_DATA_LENGTH MW_BUFLEN
static char __attribute__((aligned(2))) cmd_buf[MW_BUFLEN];

static bool mwDetected = false;
static bool recvData = false;

#define REUSE_PAYLOAD_HEADER_LEN 6
#define RECEIVER_FEEDBACK_FRAME_FREQUENCY 10

static char recvBuffer[MAX_UDP_DATA_LENGTH];
static char sendBuffer[MAX_UDP_DATA_LENGTH];
static bool awaitingRecv = false;
static bool awaitingSend = false;

#define FPS 60
#define MS_TO_FRAMES(ms) (((ms)*FPS / 500 + 1) / 2)

static MegaWifiStatus status;

static void recv_complete_cb(
    enum lsd_status stat, uint8_t ch, char* data, uint16_t len, void* ctx);

static enum mw_err associateAp(void)
{
    int16_t def_ap = mw_def_ap_cfg_get();
    def_ap = def_ap < 0 ? 0 : def_ap;

    enum mw_err err = mw_ap_assoc(def_ap);
    if (err != MW_ERR_NONE) {
        return err;
    }
    err = mw_ap_assoc_wait(MS_TO_FRAMES(20000));
    if (err != MW_ERR_NONE) {
        return err;
    }
    return MW_ERR_NONE;
}

static enum mw_err displayLocalIp(void)
{
    struct mw_ip_cfg* ip_cfg;
    enum mw_err err = mw_ip_current(&ip_cfg);
    if (err != MW_ERR_NONE) {
        return err;
    }
    char ip_str[16] = {};
    uint32_to_ip_str(ip_cfg->addr.addr, ip_str);
#if DEBUG_MEGAWIFI_INIT
    log_info("MW: IP: %s", ip_str);
#endif
    return err;
}

static bool detect_mw(void)
{
    u8 ver_major = 0, ver_minor = 0;
    char* variant = NULL;

    enum mw_err err = mw_detect(&ver_major, &ver_minor, &variant);
    if (MW_ERR_NONE != err) {
        if (settings_debug_megawifi_init()) {
            log_warn("MW: Not found");
        }
        return false;
    }
    if (settings_debug_megawifi_init()) {
        log_info("MW: Detected v%d.%d", ver_major, ver_minor);
    }
    return true;
}

static enum mw_err listenOnUdpPort(u8 ch, u16 src_port)
{
    char src_port_str[6];
    v_sprintf(src_port_str, "%u", src_port);
    enum mw_err err = mw_udp_set(ch, NULL, NULL, src_port_str);
    if (err != MW_ERR_NONE) {
        log_warn("MW: Cannot open UDP %s", src_port_str);
    }
    return err;
}

static void idle_tsk(void)
{
    while (1) {
        mw_process();
    }
}

static void tasking_init(void)
{
    TSK_userSet(idle_tsk);
}

void comm_megawifi_init(void)
{
    scheduler_addTickHandler(comm_megawifi_tick);
    scheduler_addFrameHandler(comm_megawifi_vsync);

    status = NotDetected;
    enum mw_err err = mw_init(cmd_buf, MW_BUFLEN);
    if (err != MW_ERR_NONE) {
        return;
    }
    tasking_init();

    mwDetected = detect_mw();
    if (!mwDetected) {
        return;
    }
    status = Detected;

    associateAp();
    displayLocalIp();
    err = listenOnUdpPort(CH_CONTROL_PORT, UDP_CONTROL_PORT);
    if (err != MW_ERR_NONE) {
        return;
    }
#if DEBUG_MEGAWIFI_INIT
    log_info("MW: Control UDP port %u open", UDP_CONTROL_PORT);
#endif
    err = listenOnUdpPort(CH_MIDI_PORT, UDP_MIDI_PORT);
    if (err != MW_ERR_NONE) {
        return;
    }
#if DEBUG_MEGAWIFI_INIT
    log_info("MW: MIDI UDP port %u open", UDP_MIDI_PORT);
#endif
    status = Listening;
}

u8 comm_megawifi_read_ready(void)
{
    if (!recvData)
        return false;
    return buffer_can_read();
}

u8 comm_megawifi_read(void)
{
    return buffer_read();
}

u8 comm_megawifi_write_ready(void)
{
    return 0;
}

void comm_megawifi_write(u8 data)
{
    (void)data;
}

static void processUdpData(u8 ch, char* buffer, u16 length)
{
    (void)buffer;
    enum mw_err err = MW_ERR_NONE;
    switch (ch) {
    case CH_CONTROL_PORT:
        err = applemidi_processSessionControlPacket(buffer, length);
        break;
    case CH_MIDI_PORT:
        err = applemidi_processSessionMidiPacket(buffer, length);
        break;
    }
    if (err != MW_ERR_NONE) {
        log_warn("MW: processUdpData() = %d", err);
    }
}

static u32 remoteIp = 0;
static u16 remoteControlPort = 0;
static u16 remoteMidiPort = 0;

static void persistRemoteEndpoint(u8 ch, u32 ip, u16 port)
{
    remoteIp = ip;
    if (ch == CH_CONTROL_PORT) {
        remoteControlPort = port;
    } else if (ch == CH_MIDI_PORT) {
        remoteMidiPort = port;
    }
}

static void restoreRemoteEndpoint(u8 ch, u32* ip, u16* port)
{
    *ip = remoteIp;
    *port = (ch == CH_CONTROL_PORT) ? remoteControlPort : remoteMidiPort;
}

static void recv_complete_cb(
    enum lsd_status stat, uint8_t ch, char* data, uint16_t len, void* ctx)
{
    (void)ctx;

    if (LSD_STAT_COMPLETE == stat) {
        struct mw_reuse_payload* udp = (struct mw_reuse_payload*)data;
#if DEBUG_MEGAWIFI_INIT
        char remote_ip_str[16] = {};
        uint32_to_ip_str(udp->remote_ip, remote_ip_str);
        log_info("MW: Remote=%s:%u", remote_ip_str, udp->remote_port);
#endif
        persistRemoteEndpoint(ch, udp->remote_ip, udp->remote_port);
        processUdpData(ch, udp->payload, len);
    } else {
        log_warn("MW: recv_complete_cb() = %d", stat);
    }
    awaitingRecv = false;
}

static u16 frame = 0;

void comm_megawifi_vsync(void)
{
    frame++;
}

static u16 lastSeqNum = 0;

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
    if (!mwDetected)
        return;
    mw_process();
    if (awaitingRecv || awaitingSend) {
        return;
    }
    sendReceiverFeedback();

    awaitingRecv = true;
    struct mw_reuse_payload* pkt = (struct mw_reuse_payload* const)recvBuffer;
    enum lsd_status stat
        = mw_udp_reuse_recv(pkt, MW_BUFLEN, NULL, recv_complete_cb);
    if (stat < 0) {
        log_warn("MW: mw_udp_reuse_recv() = %d", stat);
        awaitingRecv = false;
        return;
    }
}

void comm_megawifi_midiEmitCallback(u8 data)
{
    recvData = true;
    if (!buffer_can_write()) {
        log_warn("MW: MIDI buffer full!");
        return;
    }
    buffer_write(data);
}

void send_complete_cb(enum lsd_status stat, void* ctx)
{
    (void)ctx;
    if (stat < 0) {
        log_warn("MW: send_complete_cb() = %d", stat);
    }
    awaitingSend = false;
}

void comm_megawifi_send(u8 ch, char* data, u16 len)
{
    struct mw_reuse_payload* udp = (struct mw_reuse_payload*)sendBuffer;
    restoreRemoteEndpoint(ch, &udp->remote_ip, &udp->remote_port);
    memcpy(udp->payload, data, len);

    status = Connected;

#if DEBUG_MEGAWIFI_SEND == 1
    char ip_buf[16];
    uint32_to_ip_str(remoteIp, ip_buf);
    log_info("MW: Send IP=%s:%u L=%d C=%d", ip_buf, udp->remote_port, len, ch);
#endif

    awaitingSend = true;
    enum lsd_status stat = mw_udp_reuse_send(
        ch, udp, len + REUSE_PAYLOAD_HEADER_LEN, NULL, send_complete_cb);
    if (stat < 0) {
        log_warn("MW: mw_udp_reuse_send() = %d", stat);
        awaitingSend = false;
        return;
    }
}

MegaWifiStatus comm_megawifi_status(void)
{
    return status;
}
