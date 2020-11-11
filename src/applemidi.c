#include "applemidi.h"
#include "mediator.h"
#include "rtpmidi.h"
#include "vstring.h"

#include <stdbool.h>

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

static void sendInviteResponse(u8 ch, AppleMidiExchangePacket* invite)
{
    char buffer[UDP_PKT_BUFFER_LEN];
    u16 length;
    packInvitationResponse(invite->initToken, buffer, &length);
    mediator_send(ch, buffer, length);
}

static mw_err unpackTimestampSync(
    char* buffer, u16 length, AppleMidiTimeSyncPacket* timeSyncPacket)
{
    if (length != TIMESYNC_PKT_LEN) {
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

static mw_err sendTimestampSync(AppleMidiTimeSyncPacket* timeSyncPacket)
{
    char buffer[TIMESYNC_PKT_LEN];
    u16 length;
    packTimestampSync(timeSyncPacket, buffer, &length);
    mw_err err = mw_send_sync(CH_MIDI_PORT, buffer, length, 0);
    if (err != MW_ERR_NONE) {
        return err;
    }
    return MW_ERR_NONE;
}

static mw_err processInvitation(u8 ch, char* buffer, u16 length)
{
    AppleMidiExchangePacket packet;
    mw_err err = unpackInvitation(buffer, length, &packet);
    if (err != MW_ERR_NONE) {
        return err;
    }
    char text[100];
    v_sprintf(text, "Invite recv'd on UDP ch %d:", ch);
    //  VDP_drawText(text, 1, 2 + ch);

    sendInviteResponse(ch, &packet);
    v_sprintf(text, "Ack'd");
    // VDP_drawText(text, 28, 2 + ch);

    return MW_ERR_NONE;
}

static u16 timestampSyncCount = 0;

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
        err = sendTimestampSync(&packet);
        if (err != MW_ERR_NONE) {
            return err;
        }

        char text[32];
        v_sprintf(text, "Timestamp Syncs: %d", timestampSyncCount++);
        // VDP_drawText(text, 1, 5);
    }

    return MW_ERR_NONE;
}

static bool hasAppleMidiSignature(char* buffer, u16 length)
{
    if (length < 2) {
        return false;
    }
    return *((u16*)buffer) == APPLE_MIDI_SIGNATURE;
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
            //   VDP_drawText(text, 1, 14);
        }
    } else {
        return rtpmidi_processRtpMidiPacket(buffer, length);
    }

    return MW_ERR_NONE;
}
