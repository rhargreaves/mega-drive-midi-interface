#include "comm_megawifi.h"
#include "applemidi.h"
#include "log.h"
#include "mw_uart.h"
#include "settings.h"
#include "ring_buf.h"
#include "ip_util.h"
#include "scheduler.h"

#define UDP_CONTROL_PORT 5006
#define UDP_MIDI_PORT (UDP_CONTROL_PORT + 1)

#define MW_BUFLEN 1460
#define MAX_UDP_DATA_LENGTH MW_BUFLEN
static u16 cmd_buf[MW_BUFLEN];
static bool recvData = false;
static bool listening = false;
static bool connected = false;

#define REUSE_PAYLOAD_HEADER_LEN 6
#define RECEIVER_FEEDBACK_FRAME_FREQUENCY 10

static char recvBuffer[MAX_UDP_DATA_LENGTH];
static char sendBuffer[MAX_UDP_DATA_LENGTH];
static bool awaitingRecv = false;
static bool awaitingSend = false;

#define FPS 60
#define MS_TO_FRAMES(ms) (((ms) * FPS / 500 + 1) / 2)

static void recv_complete_cb(enum lsd_status stat, uint8_t ch, char* data, uint16_t len, void* ctx);

static enum mw_err associate_ap(void)
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

static enum mw_err get_local_ip(u32* ip)
{
    struct mw_ip_cfg* ip_cfg;
    enum mw_err err = mw_ip_current(&ip_cfg);
    if (err != MW_ERR_NONE) {
        return err;
    }
    *ip = ip_cfg->addr.addr;
    return MW_ERR_NONE;
}

static bool detect_mw(void)
{
    u8 ver_major = 0, ver_minor = 0;
    char* variant = NULL;

    log_info("MW: Detecting...");
    enum mw_err err = mw_detect(&ver_major, &ver_minor, &variant);
    if (MW_ERR_NONE != err) {
        log_warn("MW: Not found");
        return false;
    }
    log_info("MW: Found v%d.%d", ver_major, ver_minor);
    scheduler_yield();
    return true;
}

__attribute__((optimize("O1"))) static enum mw_err listen_on_udp_port(u8 ch, u16 src_port)
{
    char src_port_str[6];
    sprintf(src_port_str, "%u", src_port);
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

static void init_mega_wifi(void)
{
    enum mw_err err = mw_init(cmd_buf, MW_BUFLEN);
    if (err != MW_ERR_NONE) {
        log_warn("MW: Init Error %d", err);
        return;
    }
    tasking_init();

    if (!detect_mw()) {
        return;
    }
    scheduler_yield();
    associate_ap();

    u32 ip;
    err = get_local_ip(&ip);
    if (err != MW_ERR_NONE) {
        log_warn("MW: Cannot get IP");
        return;
    }
    err = listen_on_udp_port(CH_CONTROL_PORT, UDP_CONTROL_PORT);
    if (err != MW_ERR_NONE) {
        return;
    }
    err = listen_on_udp_port(CH_MIDI_PORT, UDP_MIDI_PORT);
    if (err != MW_ERR_NONE) {
        return;
    }
    listening = true;

    char ip_str[16];
    uint32_to_ip_str(ip, ip_str);
    log_info("MW: Ctrl UDP %s:%u", ip_str, UDP_CONTROL_PORT);
    scheduler_yield();
}

void comm_megawifi_init(void)
{
    listening = false;
    connected = false;
    scheduler_addTickHandler(comm_megawifi_tick);
    scheduler_addFrameHandler(comm_megawifi_vsync);

    if (comm_megawifi_is_present()) {
        scheduler_yield();
        init_mega_wifi();
    } else {
        scheduler_yield();
    }
}

bool comm_megawifi_is_present(void)
{
    return mw_uart_is_present();
}

u8 comm_megawifi_read_ready(void)
{
    if (!recvData)
        return false;
    return ring_buf_can_read();
}

u8 comm_megawifi_read(void)
{
    u8 data = 0;
    ring_buf_status_t status = ring_buf_read(&data);
    if (status == RING_BUF_OK) {
        return data;
    }

    if (status == RING_BUF_EMPTY) {
        log_warn("MW: Attempted read from empty buffer");
    }

    return 0;
}

u8 comm_megawifi_write_ready(void)
{
    return 0;
}

void comm_megawifi_write(u8 data)
{
    (void)data;
}

static void process_udp_data(u8 ch, char* buffer, u16 length)
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

static void persist_remote_endpoint(u8 ch, u32 ip, u16 port)
{
    remoteIp = ip;
    if (ch == CH_CONTROL_PORT) {
        remoteControlPort = port;
    } else if (ch == CH_MIDI_PORT) {
        remoteMidiPort = port;
    }
}

static void restore_remote_endpoint(u8 ch, u32* ip, u16* port)
{
    *ip = remoteIp;
    *port = (ch == CH_CONTROL_PORT) ? remoteControlPort : remoteMidiPort;
}

static void recv_complete_cb(enum lsd_status stat, uint8_t ch, char* data, uint16_t len, void* ctx)
{
    (void)ctx;

    if (LSD_STAT_COMPLETE == stat) {
        struct mw_reuse_payload* udp = (struct mw_reuse_payload*)data;
#if DEBUG_MEGAWIFI_SEND
        char remote_ip_str[16] = {};
        uint32_to_ip_str(udp->remote_ip, remote_ip_str);
        log_info("MW: Remote=%s:%u", remote_ip_str, udp->remote_port);
#endif
        persist_remote_endpoint(ch, udp->remote_ip, udp->remote_port);
        process_udp_data(ch, udp->payload, len);
    } else {
        log_warn("MW: recv_complete_cb() = %d", stat);
    }
    awaitingRecv = false;
}

static u16 frame = 0;

void comm_megawifi_vsync(u16 delta)
{
    frame += delta;
}

static u16 lastSeqNum = 0;

static void send_receiver_feedback(void)
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
    if (!listening) {
        return;
    }
    mw_process();
    if (awaitingRecv || awaitingSend) {
        return;
    }
    send_receiver_feedback();

    awaitingRecv = true;
    struct mw_reuse_payload* pkt = (struct mw_reuse_payload* const)recvBuffer;
    enum lsd_status stat = mw_udp_reuse_recv(pkt, MW_BUFLEN, NULL, recv_complete_cb);
    if (stat < 0) {
        log_warn("MW: mw_udp_reuse_recv() = %d", stat);
        awaitingRecv = false;
        return;
    }
}

void comm_megawifi_midiEmitCallback(u8 data)
{
    recvData = true;
    ring_buf_status_t status = ring_buf_write(data);
    if (status == RING_BUF_FULL) {
        log_warn("MW: MIDI buffer full!");
    }
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
    restore_remote_endpoint(ch, &udp->remote_ip, &udp->remote_port);
    memcpy(udp->payload, data, len);

    if (!connected) {
        log_info("MW: Session connected");
        connected = true;
    }

#if DEBUG_MEGAWIFI_SEND == 1
    char ip_buf[16];
    uint32_to_ip_str(remoteIp, ip_buf);
    log_info("MW: Send IP=%s:%u L=%d C=%d", ip_buf, udp->remote_port, len, ch);
#endif

    awaitingSend = true;
    enum lsd_status stat
        = mw_udp_reuse_send(ch, udp, len + REUSE_PAYLOAD_HEADER_LEN, NULL, send_complete_cb);
    if (stat < 0) {
        log_warn("MW: mw_udp_reuse_send() = %d", stat);
        awaitingSend = false;
        return;
    }
}