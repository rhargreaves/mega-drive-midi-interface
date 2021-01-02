#include "applemidi.h"
#include "rtpmidi.h"
#include "vstring.h"
#include "log.h"
#include <stdbool.h>
#include "comm_megawifi.h"
#include "memory.h"

static mw_err unpackInvitation(
    char* buffer, u16 length, AppleMidiExchangePacket* invite);
static void sendInviteResponse(u8 ch, AppleMidiExchangePacket* invite);

static mw_err processInvitation(u8 ch, char* buffer, u16 length)
{
    AppleMidiExchangePacket packet;
    mw_err err = unpackInvitation(buffer, length, &packet);
    if (err != MW_ERR_NONE) {
        return err;
    }
    log_info("AM: Session invite on UDP ch %d", ch);
    sendInviteResponse(ch, &packet);
    return MW_ERR_NONE;
}

static mw_err unpackInvitation(
    char* buffer, u16 length, AppleMidiExchangePacket* invite)
{
    if (length < APPLE_MIDI_EXCH_PKT_MIN_LEN) {
        return ERR_APPLE_MIDI_EXCH_PKT_TOO_SMALL;
    }

    u8 index = 0;
    while (index < EXCHANGE_PACKET_LEN) {
        invite->byte[index] = buffer[index];
        index++;
    }
    if (invite->signature != APPLE_MIDI_SIGNATURE) {
        return ERR_INVALID_APPLE_MIDI_SIGNATURE;
    }

    return MW_ERR_NONE;
}

static void packInvitationResponse(u32 initToken, char* buffer, u16* length)
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

static char inviteSendBuffer[UDP_PKT_BUFFER_LEN];

static void sendInviteResponse(u8 ch, AppleMidiExchangePacket* invite)
{
    u16 length;
    packInvitationResponse(invite->initToken, inviteSendBuffer, &length);
    comm_megawifi_send(ch, inviteSendBuffer, length);
}

static mw_err unpackTimestampSync(
    char* buffer, u16 length, AppleMidiTimeSyncPacket* timeSyncPacket)
{
    if (length < TIMESYNC_PKT_LEN) {
        return ERR_INVALID_TIMESYNC_PKT_LENGTH;
    }
    u8 index = 0;
    while (index < TIMESYNC_PKT_LEN) {
        timeSyncPacket->byte[index] = buffer[index];
        index++;
    }
    if (timeSyncPacket->signature != APPLE_MIDI_SIGNATURE) {
        return ERR_INVALID_APPLE_MIDI_SIGNATURE;
    }
    return MW_ERR_NONE;
}

static void packTimestampSync(
    AppleMidiTimeSyncPacket* timeSyncPacket, char* buffer, u16* length)
{
    *length = 0;
    while (*length < TIMESYNC_PKT_LEN) {
        buffer[*length] = timeSyncPacket->byte[*length];
        (*length)++;
    }
}

char timestampSyncSendBuffer[TIMESYNC_PKT_LEN];

static void sendTimestampSync(AppleMidiTimeSyncPacket* timeSyncPacket)
{
    u16 length;
    packTimestampSync(timeSyncPacket, timestampSyncSendBuffer, &length);
    comm_megawifi_send(CH_MIDI_PORT, timestampSyncSendBuffer, length);
}

static mw_err processTimestampSync(char* buffer, u16 length)
{
    AppleMidiTimeSyncPacket packet;
    mw_err err = unpackTimestampSync(buffer, length, &packet);
    if (err != MW_ERR_NONE) {
        return err;
    }
    if (packet.count == 0) {
        packet.count = 1;
        packet.timestamp2Hi = 0;
        packet.timestamp2Lo = 0;
        packet.senderSSRC = MEGADRIVE_SSRC;
        log_info("AM: Timestamp Sync");
        sendTimestampSync(&packet);
    }

    return MW_ERR_NONE;
}

static bool hasAppleMidiSignature(char* buffer, u16 length)
{
    if (length < 2) {
        return false;
    }
    return (u8)buffer[0] == 0xFF && (u8)buffer[1] == 0xFF;
}

static bool isInvitationCommand(char* command)
{
    return command[0] == 'I' && command[1] == 'N';
}

static bool isTimestampSyncCommand(char* command)
{
    return command[0] == 'C' && command[1] == 'K';
}

mw_err applemidi_processSessionControlPacket(char* buffer, u16 length)
{
    if (!hasAppleMidiSignature(buffer, length)) {
        return ERR_INVALID_APPLE_MIDI_SIGNATURE;
    }
    char* command = &buffer[2];
    if (isInvitationCommand(command)) {
        return processInvitation(CH_CONTROL_PORT, buffer, length);
    }

    return MW_ERR_NONE;
}

static u16 lastSeqNum = 0;

mw_err applemidi_processSessionMidiPacket(char* buffer, u16 length)
{
    if (hasAppleMidiSignature(buffer, length)) {
        char* command = &buffer[2];
        if (isInvitationCommand(command)) {
            return processInvitation(CH_MIDI_PORT, buffer, length);
        } else if (isTimestampSyncCommand(command)) {
            return processTimestampSync(buffer, length);
        } else {
            char text[100];
            v_sprintf(text, "Unknown event %s", command);
        }
    } else {
        return rtpmidi_processRtpMidiPacket(buffer, length, &lastSeqNum);
    }

    return MW_ERR_NONE;
}

u16 applemidi_lastSequenceNumber(void)
{
    return lastSeqNum;
}

#define RECEIVER_FEEDBACK_PACKET_LENGTH 12

mw_err applemidi_sendReceiverFeedback(void)
{
    char receiverFeedbackPacket[RECEIVER_FEEDBACK_PACKET_LENGTH]
        = { 0xFF, 0xFF, 'R', 'S', (u8)(MEGADRIVE_SSRC >> 24),
              (u8)(MEGADRIVE_SSRC >> 16), (u8)(MEGADRIVE_SSRC >> 8),
              (u8)MEGADRIVE_SSRC, (u8)(lastSeqNum >> 8), (u8)lastSeqNum, 0, 0 };

    comm_megawifi_send(CH_CONTROL_PORT, receiverFeedbackPacket,
        RECEIVER_FEEDBACK_PACKET_LENGTH);
    return MW_ERR_NONE;
}
