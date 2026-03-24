#include "comm_megawifi.h"
#include "applemidi.h"
#include "rtpmidi.h"
#include "log.h"
#include "mw_uart.h"
#include "settings.h"
#include "ring_buf.h"
#include "ip_util.h"
#include "scheduler.h"

#define UDP_CONTROL_PORT 5006
#define UDP_MIDI_PORT (UDP_CONTROL_PORT + 1)

#define REUSE_PAYLOAD_HEADER_LEN 6
#define RECEIVER_FEEDBACK_FRAME_FREQUENCY 10
#define SEND_RTP_MIDI_PACKET_BUFFER_LEN 256
#define FPS 60
#define MS_TO_FRAMES(ms) (((ms) * FPS / 500 + 1) / 2)

static u16 cmd_buf[MW_MSG_MAX_BUFLEN / sizeof(u16)];
static bool recvData;
static u8 ring_buf_arr[MW_RX_TX_BUFFER_SIZE];
static ring_buf_t ring_buf;
static bool listening;
static bool connected;
static u16 frame;
static u16 lastSeqNum;

static u32 remoteIp;
static u16 remoteControlPort;
static u16 remoteMidiPort;

typedef enum { RX_IDLE, RX_ARMED, RX_BUFFERED } rx_state;
typedef enum { TX_IDLE, TX_QUEUED, TX_INFLIGHT } tx_state;

static rx_state rxState;
static int rxCh;
static int rxLen;
static char rxBuffer[MW_RX_TX_BUFFER_SIZE];

static tx_state txState;
static int txCh;
static int txLen;
static char txBuffer[MW_RX_TX_BUFFER_SIZE];

static void init_mega_wifi(void);
static void recv_complete_cb(enum lsd_status stat, uint8_t ch, char* data, uint16_t len, void* ctx);
static void send_buffered_packet(void);

void comm_megawifi_init(void)
{
    rtpmidi_init();

    memset(cmd_buf, 0, sizeof(cmd_buf));
    ring_buf_init(&ring_buf, ring_buf_arr, sizeof(ring_buf_arr));
    recvData = false;
    listening = false;
    connected = false;
    frame = 0;
    lastSeqNum = 0;
    memset(rxBuffer, 0, sizeof(rxBuffer));
    memset(txBuffer, 0, sizeof(txBuffer));
    remoteIp = 0;
    remoteControlPort = 0;
    remoteMidiPort = 0;

    rxState = RX_IDLE;
    rxCh = 0;
    rxLen = 0;
    txState = TX_IDLE;
    txCh = 0;
    txLen = 0;

    scheduler_addTickHandler(comm_megawifi_tick);
    scheduler_addFrameHandler(comm_megawifi_vsync);

    if (comm_megawifi_is_present()) {
        scheduler_yield();
        init_mega_wifi();
    } else {
        scheduler_yield();
    }
}

static bool has_apple_midi_signature(char* buffer, u16 length)
{
    if (length < 2) {
        return false;
    }
    return (u8)buffer[0] == 0xFF && (u8)buffer[1] == 0xFF;
}

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
    enum mw_err err = mw_init(cmd_buf, sizeof(cmd_buf));
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
    log_info("MW: IP=%s:%u", ip_str, UDP_CONTROL_PORT);
    scheduler_yield();
}

bool comm_megawifi_is_present(void)
{
    return mw_uart_is_present();
}

u8 comm_megawifi_read_ready(void)
{
    if (!recvData)
        return false;
    return ring_buf_can_read(&ring_buf);
}

u8 comm_megawifi_read(void)
{
    u8 data = 0;
    ring_buf_status_t status = ring_buf_read(&ring_buf, &data);
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
    return true;
}

void comm_megawifi_write(const u8* data, u16 length)
{
    u8 buffer[SEND_RTP_MIDI_PACKET_BUFFER_LEN + length];
    u16 packedLength = rtpmidi_packRtpMidiPacket(data, length, buffer, sizeof(buffer));
    if (packedLength == 0) {
        return;
    }
    comm_megawifi_send(CH_MIDI_PORT, (char*)buffer, packedLength);
}

static void process_udp_data(u8 ch, char* buffer, u16 length)
{
    switch (ch) {
    case CH_CONTROL_PORT:
        applemidi_processSessionControlPacket(buffer, length);
        break;
    case CH_MIDI_PORT:
        if (has_apple_midi_signature(buffer, length)) {
            applemidi_processSessionMidiPacket(buffer, length);
        } else {
            rtpmidi_processRtpMidiPacket(buffer, length);
        }
        break;
    }
}

static void persist_remote_endpoint(u8 ch, u32 ip, u16 port)
{
    remoteIp = ip;
    if (ch == CH_CONTROL_PORT) {
        remoteControlPort = port;
        remoteMidiPort = port + 1;
    } else if (ch == CH_MIDI_PORT) {
        remoteControlPort = port - 1;
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

    if (rxState != RX_ARMED) {
        log_warn("MW: rxCb unexpected state: %d", rxState);
        return;
    }

    if (LSD_STAT_COMPLETE == stat) {
        rxState = RX_BUFFERED;
        rxCh = ch;
        rxLen = len - REUSE_PAYLOAD_HEADER_LEN;

        struct mw_reuse_payload* udp = (struct mw_reuse_payload*)data;
        persist_remote_endpoint(ch, udp->remote_ip, udp->remote_port);

#if DEBUG_MEGAWIFI_RECV
        char remote_ip_str[16] = {};
        uint32_to_ip_str(udp->remote_ip, remote_ip_str);
        log_info("MW: R=%s:%u L=%d C=%d", remote_ip_str, udp->remote_port, rxLen, rxCh);
#endif
    } else {
        log_warn("MW: rxCb = %d", stat);
        rxState = RX_IDLE;
    }
}

void comm_megawifi_vsync(u16 delta)
{
    frame += delta;
}

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

    if (rxState == RX_BUFFERED) {
        process_udp_data(rxCh, rxBuffer + REUSE_PAYLOAD_HEADER_LEN, rxLen);
        rxState = RX_IDLE;
    }

    if (txState == TX_QUEUED) {
        send_buffered_packet();
    }

    if (txState == TX_IDLE) {
        send_receiver_feedback();
    }

    if (rxState == RX_IDLE) {
        struct mw_reuse_payload* pkt = (struct mw_reuse_payload* const)rxBuffer;
        enum lsd_status stat = mw_udp_reuse_recv(pkt, MW_RX_TX_BUFFER_SIZE, NULL, recv_complete_cb);
        if (stat < 0) {
            log_warn("MW: mw_udp_reuse_recv() = %d", stat);
            return;
        }
        rxState = RX_ARMED;
    }
}

void comm_megawifi_midiEmitCallback(u8 data)
{
    recvData = true;
    ring_buf_status_t status = ring_buf_write(&ring_buf, data);
    if (status == RING_BUF_FULL) {
        log_warn("MW: MIDI buffer full!");
    }
}

void send_complete_cb(enum lsd_status stat, void* ctx)
{
    (void)ctx;
    if (stat < LSD_STAT_COMPLETE) {
        log_warn("MW: send_complete_cb() = %d", stat);
    }

    txState = TX_IDLE;
}

static void send_buffered_packet(void)
{
    if (txState != TX_QUEUED) {
        return;
    }

    u32 ip;
    u16 port;
    restore_remote_endpoint(txCh, &ip, &port);
    txBuffer[0] = (u8)(ip >> 24);
    txBuffer[1] = (u8)(ip >> 16);
    txBuffer[2] = (u8)(ip >> 8);
    txBuffer[3] = (u8)(ip & 0xFF);
    txBuffer[4] = (u8)(port >> 8);
    txBuffer[5] = (u8)(port & 0xFF);

    if (!connected) {
        log_info("MW: Session connected");
        connected = true;
    }

#if DEBUG_MEGAWIFI_SEND == 1
    char ip_buf[16];
    uint32_to_ip_str(remoteIp, ip_buf);
    log_info("MW: S=%s:%u L=%d C=%d", ip_buf, port, txLen, txCh);
#endif

    txState = TX_INFLIGHT;
    enum lsd_status stat = mw_udp_reuse_send(txCh, (struct mw_reuse_payload*)txBuffer,
        txLen + REUSE_PAYLOAD_HEADER_LEN, NULL, send_complete_cb);
    if (stat < 0) {
        log_warn("MW: mw_udp_reuse_send() = %d", stat);
        txState = TX_IDLE;
        return;
    }
}

void comm_megawifi_send(u8 ch, char* data, u16 len)
{
    if (txState != TX_IDLE) {
        log_warn("MW: Send while TX pending!");
        return;
    }

    if (len + REUSE_PAYLOAD_HEADER_LEN > MW_RX_TX_BUFFER_SIZE) {
        log_warn("MW: TX buffer overflow!");
        return;
    }

    txState = TX_QUEUED;
    txCh = ch;
    txLen = len;
    memcpy(&txBuffer[REUSE_PAYLOAD_HEADER_LEN], data, len);
}
