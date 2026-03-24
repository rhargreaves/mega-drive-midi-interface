#pragma once
#include "genesis.h"
#include "utils.h"

typedef enum {
    MIDI_PKT_OK = 0,
    MIDI_PKT_INVALID_SIGNATURE,
    MIDI_PKT_EXCH_TOO_SMALL,
    MIDI_PKT_INVALID_TIMESYNC_LENGTH,
    MIDI_PKT_RTP_HEADER_TOO_SHORT,
    MIDI_PKT_RTP_LENGTH_MISMATCH,
    MIDI_PKT_UNSUPPORTED_COMMAND,
} midi_pkt_result;

typedef enum {
    APPLEMIDI_CTRL_EVENT_NONE = 0,
    APPLEMIDI_CTRL_EVENT_SESSION_END,
} applemidi_control_event;

#define MEGADRIVE_SSRC 0x9E915150
#define CH_CONTROL_PORT 1
#define CH_MIDI_PORT 2

#define NAME_LEN 16

#define RTP_MIDI_COMMAND_SECTION_HEADER_MAX_LEN 2
#define RTP_MIDI_HEADER_LEN (3 * 4)
#define EXCHANGE_PACKET_LEN (16 + NAME_LEN)
#define UDP_PKT_BUFFER_LEN 64
#define APPLE_MIDI_EXCH_PKT_MIN_LEN 17

#define TIMESYNC_PKT_LEN (9 * 4)

#define APPLE_MIDI_SIGNATURE 0xFFFF

union PACK_BIG_ENDIAN AppleMidiTimeSyncPacket {
    u8 byte[TIMESYNC_PKT_LEN];
    struct {
        u16 signature;
        char ck[2];
        u32 senderSSRC;
        u8 count;
        u8 padding[3];
        u32 timestamp1Hi;
        u32 timestamp1Lo;
        u32 timestamp2Hi;
        u32 timestamp2Lo;
        u32 timestamp3Hi;
        u32 timestamp3Lo;
    };
};

typedef union AppleMidiTimeSyncPacket AppleMidiTimeSyncPacket;

union PACK_BIG_ENDIAN AppleMidiExchangePacket {
    u8 byte[EXCHANGE_PACKET_LEN];
    struct {
        u16 signature;
        char command[2];
        u32 version;
        u32 initToken;
        u32 senderSSRC;
        char name[NAME_LEN];
    };
};

typedef union AppleMidiExchangePacket AppleMidiExchangePacket;

midi_pkt_result applemidi_processSessionControlPacket(
    char* buffer, u16 length, applemidi_control_event* event);
midi_pkt_result applemidi_processSessionMidiPacket(char* buffer, u16 length);
u16 applemidi_lastSequenceNumber(void);
void applemidi_updateLastSeqNum(u16 seqNum);
enum mw_err applemidi_sendReceiverFeedback(void);
void applemidi_getLocalTimestamp(u32* timestampHi, u32* timestampLo);
