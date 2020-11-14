#include "rtpmidi.h"
#include "comm_megawifi.h"

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
    if (STATUS_UPPER(status) == 0xC || STATUS_UPPER(status) == 0xD
        || status == 0xF1 || status == 0xF3) {
        return 1;
    } else {
        return 2;
    }
}

static void emitMidiEvent(u8 status, u8** cursor)
{
    comm_megawifi_midiEmitCallback(status);
    for (u8 i = 0; i < bytesToEmit(status); i++) {
        comm_megawifi_midiEmitCallback(**cursor);
        (*cursor)++;
    };
}

#define MIDI_SYSEX_START 0xF0
#define MIDI_SYSEX_END 0xF7

void processSysEx(u8** cursor)
{
    do {
        comm_megawifi_midiEmitCallback(**cursor);
        (*cursor)++;
    } while (**cursor != MIDI_SYSEX_END);
    comm_megawifi_midiEmitCallback(**cursor);
}

mw_err rtpmidi_processRtpMidiPacket(char* buffer, u16 length)
{
    (void)length; // TODO: Probably shouldn't ignore length...
    u8* commandSection = (u8*)&buffer[RTP_MIDI_HEADER_LEN];
    bool longHeader = isLongHeader(commandSection);
    u16 midiLength = longHeader ? twelveBitMidiLength(commandSection)
                                : fourBitMidiLength(commandSection);
    u8* midiStart = &commandSection[longHeader ? 2 : 1];
    u8* midiEnd = &midiStart[midiLength];
    u8 status = 0;
    u8* cursor = midiStart;
    while (cursor != midiEnd) {

        if (*cursor == MIDI_SYSEX_START) {
            processSysEx(&cursor);

            // fast forward over high delta time octets
            while (*cursor & 0x80) { cursor++; }
            // skip over final low delta time octet
            if (cursor == midiEnd) {
                break;
            }
            cursor++;
            continue;
        }

        if (*cursor == MIDI_SYSEX_END) {
            // middle sysex segment
            // we're ignoring these for now...
            while (*cursor != MIDI_SYSEX_START) { cursor++; }
            cursor++;

            // fast forward over high delta time octets
            while (*cursor & 0x80) { cursor++; }
            // skip over final low delta time octet
            cursor++;
            continue;
        }

        if (*cursor & 0x80) { // status bit present
            status = *cursor;
            cursor++;
            continue;
        }

        emitMidiEvent(status, &cursor);
        if (cursor == midiEnd) {
            break;
        }
        // fast forward over high delta time octets
        while (*cursor & 0x80) { cursor++; }
        // skip over final low delta time octet
        cursor++;
    }

    return MW_ERR_NONE;
}
