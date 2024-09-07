#include "rtpmidi.h"
#include "comm_megawifi.h"
#include "bits.h"

#define MIDI_SYSEX_START 0xF0
#define MIDI_SYSEX_END 0xF7
#define MIDI_RESET 0xFF

#define STATUS_UPPER(status) (status >> 4)

static bool isLongHeader(u8* commandSection)
{
    return (u8)commandSection[0] >> 7;
}

static u16 fourBitMidiLength(u8* commandSection)
{
    return commandSection[0] & 0x0F;
}

static u16 twelveBitMidiLength(u8* commandSection)
{
    return (((u16)commandSection[0] << 12) + (u16)commandSection[1]);
}

static u8 bytesToEmit(u8 status)
{
    if (STATUS_UPPER(status) == 0xC || STATUS_UPPER(status) == 0xD || status == 0xF1
        || status == 0xF3) {
        return 1;
    } else {
        return 2;
    }
}

static void emitMidiEvent(u8 status, u8** cursor)
{
    comm_megawifi_midiEmitCallback(status);
    u8 count = bytesToEmit(status);
    for (u8 i = 0; i < count; i++) {
        comm_megawifi_midiEmitCallback(**cursor);
        if (i < count - 1) {
            (*cursor)++;
        }
    };
}

static void processSysEx(u8** cursor)
{
    do {
        comm_megawifi_midiEmitCallback(**cursor);
        (*cursor)++;
    } while (!(**cursor == MIDI_SYSEX_END || **cursor == MIDI_SYSEX_START));
    comm_megawifi_midiEmitCallback(MIDI_SYSEX_END);
}

static void processMiddleSysEx(u8** cursor)
{
    // we're ignoring these for now...
    do {
        (*cursor)++;
    } while (!(**cursor == MIDI_SYSEX_END || **cursor == MIDI_SYSEX_START));
}

static u16 sequenceNumber(char* buffer)
{
    return ((u8)buffer[2] << 8) + (u8)buffer[3];
}

static bool isFinalDeltaByte(u8 value)
{
    return !CHECK_BIT(value, 7);
}

enum mw_err rtpmidi_processRtpMidiPacket(char* buffer, u16 length, u16* lastSeqNum)
{
    (void)length; // TODO: Probably shouldn't ignore length...
    u16 seqNum = sequenceNumber(buffer);
    u8* commandSection = (u8*)&buffer[RTP_MIDI_HEADER_LEN];
    bool longHeader = isLongHeader(commandSection);
    u16 midiLength
        = longHeader ? twelveBitMidiLength(commandSection) : fourBitMidiLength(commandSection);
    u8* midiStart = &commandSection[longHeader ? 2 : 1];
    u8* midiEnd = midiStart + (midiLength - 1);
    u8 status = 0;
    u8* cursor = midiStart;

    bool walkingOverDeltas = false;
    while (cursor <= midiEnd) {
        if (walkingOverDeltas && isFinalDeltaByte(*cursor)) {
            walkingOverDeltas = false;
        } else if (*cursor == MIDI_SYSEX_START) {
            processSysEx(&cursor);
            walkingOverDeltas = true;
        } else if (*cursor == MIDI_SYSEX_END) {
            processMiddleSysEx(&cursor);
            walkingOverDeltas = true;
        } else if (*cursor == MIDI_RESET) {
            comm_megawifi_midiEmitCallback(*cursor);
            walkingOverDeltas = true;
        } else if (CHECK_BIT(*cursor, 7)) { // status bit present
            status = *cursor;
        } else {
            emitMidiEvent(status, &cursor);
            walkingOverDeltas = true;
        }
        cursor++;
    }

    *lastSeqNum = seqNum;
    return MW_ERR_NONE;
}
