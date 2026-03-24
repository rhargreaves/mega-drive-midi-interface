#include "applemidi.h"
#include "log.h"
#include "comm_megawifi.h"
#include "settings.h"

#define RECEIVER_FEEDBACK_PACKET_LENGTH 12

static u16 lastSeqNum = 0;
static char timestampSyncSendBuffer[TIMESYNC_PKT_LEN];
static char inviteSendBuffer[UDP_PKT_BUFFER_LEN];

static midi_pkt_result unpack_invitation(char* buffer, u16 length, AppleMidiExchangePacket* invite);
static void send_invite_response(u8 ch, AppleMidiExchangePacket* invite);

static u32 localTimestampHi = 0;
static u32 localTimestampLo = 0;

void applemidi_getLocalTimestamp(u32* timestampHi, u32* timestampLo)
{
    *timestampHi = localTimestampHi;
    *timestampLo = localTimestampLo;
}

static midi_pkt_result process_invitation(u8 ch, char* buffer, u16 length)
{
    AppleMidiExchangePacket packet;
    midi_pkt_result result = unpack_invitation(buffer, length, &packet);
    if (result != MIDI_PKT_OK) {
        return result;
    }

    if (settings_debug_megawifi_init()) {
        log_info("AM: Session invite on UDP ch %d", ch);
    }
    send_invite_response(ch, &packet);
    return MIDI_PKT_OK;
}

static midi_pkt_result unpack_invitation(char* buffer, u16 length, AppleMidiExchangePacket* invite)
{
    if (length < APPLE_MIDI_EXCH_PKT_MIN_LEN) {
        return MIDI_PKT_EXCH_TOO_SMALL;
    }

    u8 index = 0;
    while (index < EXCHANGE_PACKET_LEN) {
        invite->byte[index] = buffer[index];
        index++;
    }
    if (invite->signature != APPLE_MIDI_SIGNATURE) {
        return MIDI_PKT_INVALID_SIGNATURE;
    }

    return MIDI_PKT_OK;
}

static void pack_invitation_response(u32 initToken, char* buffer, u16* length)
{
    AppleMidiExchangePacket response = { .signature = APPLE_MIDI_SIGNATURE,
        .command = "OK",
        .name = "MegaDrive",
        .initToken = initToken,
        .senderSSRC = MEGADRIVE_SSRC,
        .version = 2 };
    *length = 0;
    while (*length < EXCHANGE_PACKET_LEN) {
        buffer[*length] = response.byte[*length];
        (*length)++;
    }
}

static void send_invite_response(u8 ch, AppleMidiExchangePacket* invite)
{
    u16 length;
    pack_invitation_response(invite->initToken, inviteSendBuffer, &length);
    comm_megawifi_send(ch, inviteSendBuffer, length);
}

static midi_pkt_result unpack_timestamp_sync(
    char* buffer, u16 length, AppleMidiTimeSyncPacket* timeSyncPacket)
{
    if (length < TIMESYNC_PKT_LEN) {
        return MIDI_PKT_INVALID_TIMESYNC_LENGTH;
    }
    u8 index = 0;
    while (index < TIMESYNC_PKT_LEN) {
        timeSyncPacket->byte[index] = buffer[index];
        index++;
    }
    if (timeSyncPacket->signature != APPLE_MIDI_SIGNATURE) {
        return MIDI_PKT_INVALID_SIGNATURE;
    }
    return MIDI_PKT_OK;
}

static void pack_timestamp_sync(AppleMidiTimeSyncPacket* timeSyncPacket, char* buffer, u16* length)
{
    *length = 0;
    while (*length < TIMESYNC_PKT_LEN) {
        buffer[*length] = timeSyncPacket->byte[*length];
        (*length)++;
    }
}

static void send_timestamp_sync(AppleMidiTimeSyncPacket* timeSyncPacket)
{
    u16 length;
    pack_timestamp_sync(timeSyncPacket, timestampSyncSendBuffer, &length);
    comm_megawifi_send(CH_MIDI_PORT, timestampSyncSendBuffer, length);
}

static midi_pkt_result process_timestamp_sync(char* buffer, u16 length)
{
    AppleMidiTimeSyncPacket packet;
    midi_pkt_result result = unpack_timestamp_sync(buffer, length, &packet);
    if (result != MIDI_PKT_OK) {
        return result;
    }
    if (packet.count == 0) {
        packet.count = 1;
        packet.timestamp2Hi = packet.timestamp1Hi;
        packet.timestamp2Lo = packet.timestamp1Lo + 10;
        localTimestampHi = packet.timestamp2Hi;
        localTimestampLo = packet.timestamp2Lo;
        packet.senderSSRC = MEGADRIVE_SSRC;
#if DEBUG_MEGAWIFI_SYNC
        log_info("AM: Timestamp Sync");
#endif
        send_timestamp_sync(&packet);
    }

    return MIDI_PKT_OK;
}

static bool has_apple_midi_signature(char* buffer, u16 length)
{
    if (length < 2) {
        return false;
    }
    return (u8)buffer[0] == 0xFF && (u8)buffer[1] == 0xFF;
}

static bool is_invitation_command(char* command)
{
    return command[0] == 'I' && command[1] == 'N';
}

static bool is_timestamp_sync_command(char* command)
{
    return command[0] == 'C' && command[1] == 'K';
}

static bool is_receiver_feedback_command(char* command)
{
    return command[0] == 'R' && command[1] == 'S';
}

static bool is_session_end_command(char* command)
{
    return command[0] == 'B' && command[1] == 'Y';
}

midi_pkt_result applemidi_processSessionControlPacket(
    char* buffer, u16 length, applemidi_control_event* event)
{
    if (event != NULL) {
        *event = APPLEMIDI_CTRL_EVENT_NONE;
    }

    if (!has_apple_midi_signature(buffer, length)) {
        return MIDI_PKT_INVALID_SIGNATURE;
    }
    char* command = &buffer[2];
    if (is_invitation_command(command)) {
        return process_invitation(CH_CONTROL_PORT, buffer, length);
    } else if (is_receiver_feedback_command(command)) {
        return MIDI_PKT_OK;
    } else if (is_session_end_command(command)) {
        if (event != NULL) {
            *event = APPLEMIDI_CTRL_EVENT_SESSION_END;
        }
        return MIDI_PKT_OK;
    }

    log_warn("AM: ? ctrl cmd: %c%c", command[0], command[1]);
    return MIDI_PKT_UNSUPPORTED_COMMAND;
}

midi_pkt_result applemidi_processSessionMidiPacket(char* buffer, u16 length)
{
    if (!has_apple_midi_signature(buffer, length)) {
        return MIDI_PKT_INVALID_SIGNATURE;
    }
    char* command = &buffer[2];
    if (is_invitation_command(command)) {
        return process_invitation(CH_MIDI_PORT, buffer, length);
    } else if (is_timestamp_sync_command(command)) {
        return process_timestamp_sync(buffer, length);
    } else {
        log_warn("AM: ? midi cmd: %c%c", command[0], command[1]);
        return MIDI_PKT_UNSUPPORTED_COMMAND;
    }
}

u16 applemidi_lastSequenceNumber(void)
{
    return lastSeqNum;
}

void applemidi_updateLastSeqNum(u16 seqNum)
{
    lastSeqNum = seqNum;
}

enum mw_err applemidi_sendReceiverFeedback(void)
{
    char receiverFeedbackPacket[RECEIVER_FEEDBACK_PACKET_LENGTH] = { 0xFF, 0xFF, 'R', 'S',
        (u8)(MEGADRIVE_SSRC >> 24), (u8)(MEGADRIVE_SSRC >> 16), (u8)(MEGADRIVE_SSRC >> 8),
        (u8)MEGADRIVE_SSRC, (u8)(lastSeqNum >> 8), (u8)lastSeqNum, 0, 0 };

    comm_megawifi_send(CH_CONTROL_PORT, receiverFeedbackPacket, RECEIVER_FEEDBACK_PACKET_LENGTH);
    return MW_ERR_NONE;
}
