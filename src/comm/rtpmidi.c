#include "rtpmidi.h"
#include "applemidi.h"
#include "comm_megawifi.h"
#include "utils.h"
#include "log.h"

#define MIDI_SYSEX_START 0xF0
#define MIDI_SYSEX_END 0xF7
#define MIDI_RESET 0xFF

#define STATUS_UPPER(status) (status >> 4)
#define RTP_MIDI_PAYLOAD_TYPE 0x61
#define RTP_MIDI_SHORT_LEN_MAX 0x0F
#define RTP_MIDI_LONG_LEN_MAX 0x0FFF

#define SEQ_NUM_START 1;

static u16 sendSeqNum = SEQ_NUM_START;

void rtpmidi_resetSendState(void)
{
    sendSeqNum = SEQ_NUM_START;
}

static bool is_long_header(u8* commandSection)
{
    return (u8)commandSection[0] >> 7;
}

static u16 four_bit_midi_length(u8* commandSection)
{
    return commandSection[0] & 0x0F;
}

static u16 twelve_bit_midi_length(u8* commandSection)
{
    return (((u16)commandSection[0] << 12) + (u16)commandSection[1]);
}

static u8 bytes_to_emit(u8 status)
{
    if (STATUS_UPPER(status) == 0xC || STATUS_UPPER(status) == 0xD || status == 0xF1
        || status == 0xF3) {
        return 1;
    } else {
        return 2;
    }
}

static void emit_midi_event(u8 status, u8** cursor)
{
    comm_megawifi_midiEmitCallback(status);
    u8 count = bytes_to_emit(status);
    for (u8 i = 0; i < count; i++) {
        comm_megawifi_midiEmitCallback(**cursor);
        if (i < count - 1) {
            (*cursor)++;
        }
    };
}

static void process_sysex(u8** cursor)
{
    do {
        comm_megawifi_midiEmitCallback(**cursor);
        (*cursor)++;
    } while (!(**cursor == MIDI_SYSEX_END || **cursor == MIDI_SYSEX_START));
    comm_megawifi_midiEmitCallback(MIDI_SYSEX_END);
}

static void process_middle_sysex(u8** cursor)
{
    log_warn("rtpmidi: ignoring middle sysex");
    // we're ignoring these for now...
    do {
        (*cursor)++;
    } while (!(**cursor == MIDI_SYSEX_END || **cursor == MIDI_SYSEX_START));
}

static u16 sequence_number(char* buffer)
{
    return ((u8)buffer[2] << 8) + (u8)buffer[3];
}

static bool is_final_delta_byte(u8 value)
{
    return !CHECK_BIT(value, 7);
}

midi_pkt_result rtpmidi_processRtpMidiPacket(char* buffer, u16 length)
{
    if (length < RTP_MIDI_HEADER_LEN + 1) {
        return MIDI_PKT_RTP_HEADER_TOO_SHORT;
    }

    u16 seqNum = sequence_number(buffer);
#if DEBUG_MEGAWIFI_RECV == 1
    log_info("rtpmidi: len=%d seqNum=%d", length, seqNum);
#endif
    u8* commandSection = (u8*)&buffer[RTP_MIDI_HEADER_LEN];
    bool longHeader = is_long_header(commandSection);
    u16 midiLength = longHeader ? twelve_bit_midi_length(commandSection)
                                : four_bit_midi_length(commandSection);

    u16 headerLen = RTP_MIDI_HEADER_LEN + (longHeader ? 2 : 1);
    if (headerLen + midiLength > length) {
        return MIDI_PKT_RTP_LENGTH_MISMATCH;
    }

    u8* midiStart = &commandSection[longHeader ? 2 : 1];
    u8* midiEnd = midiStart + (midiLength - 1);
    u8 status = 0;
    u8* cursor = midiStart;

    bool walkingOverDeltas = false;
    while (cursor <= midiEnd) {
        if (walkingOverDeltas && is_final_delta_byte(*cursor)) {
            walkingOverDeltas = false;
        } else if (*cursor == MIDI_SYSEX_START) {
            process_sysex(&cursor);
            walkingOverDeltas = true;
        } else if (*cursor == MIDI_SYSEX_END) {
            process_middle_sysex(&cursor);
            walkingOverDeltas = true;
        } else if (*cursor == MIDI_RESET) {
            comm_megawifi_midiEmitCallback(*cursor);
            walkingOverDeltas = true;
        } else if (CHECK_BIT(*cursor, 7)) { // status bit present
            status = *cursor;
        } else {
            emit_midi_event(status, &cursor);
            walkingOverDeltas = true;
        }
        cursor++;
    }

    applemidi_updateLastSeqNum(seqNum);
    return MIDI_PKT_OK;
}

u16 rtpmidi_packRtpMidiPacket(const u8* midiData, u16 midiDataLength, u8* buffer, u16 bufferLength)
{
    if (midiDataLength > 0 && midiData == NULL) {
        return 0;
    }
    if (midiDataLength > RTP_MIDI_LONG_LEN_MAX) {
        return 0;
    }

    /*
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |V=2|P|X|  CC   |M|     PT      |       sequence number         |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                           timestamp                           |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                             SSRC                              |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                     MIDI command section ...                  |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                       Journal section ...                     |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */

    u32 timestampHi, timestampLo;
    applemidi_getLocalTimestamp(&timestampHi, &timestampLo);

    buffer[0] = 0x80; /* V=2 P=0 X=0 CC=0 */
    bool marker = midiDataLength > 0; /* M */
    buffer[1] = (marker ? 0x80 : 0x00) | RTP_MIDI_PAYLOAD_TYPE;
    buffer[2] = (u8)(sendSeqNum >> 8);
    buffer[3] = (u8)(sendSeqNum & 0xFF);
    buffer[4] = (u8)(timestampLo >> 24);
    buffer[5] = (u8)(timestampLo >> 16);
    buffer[6] = (u8)(timestampLo >> 8);
    buffer[7] = (u8)(timestampLo & 0xFF);
    buffer[8] = (u8)(MEGADRIVE_SSRC >> 24);
    buffer[9] = (u8)(MEGADRIVE_SSRC >> 16);
    buffer[10] = (u8)(MEGADRIVE_SSRC >> 8);
    buffer[11] = (u8)(MEGADRIVE_SSRC & 0xFF);

    /*
    MIDI command section:
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |B|J|Z|P|LEN... |  MIDI list ...                                |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */

    bool useLongHeader = midiDataLength > RTP_MIDI_SHORT_LEN_MAX; /* B */
    u16 commandHeaderLength = useLongHeader ? 2 : 1; /* LEN... */
    /* Z=0, P=0 */
    u16 packedLength = RTP_MIDI_HEADER_LEN + commandHeaderLength + midiDataLength;
    if (packedLength > bufferLength) {
        return 0;
    }

    if (useLongHeader) {
        buffer[12] = 0x80 | (u8)(midiDataLength >> 8);
        buffer[13] = (u8)(midiDataLength & 0xFF);
    } else {
        buffer[12] = (u8)(midiDataLength & 0x0F);
    }

    if (midiDataLength > 0) {
        memcpy(&buffer[RTP_MIDI_HEADER_LEN + commandHeaderLength], midiData, midiDataLength);
    }

    sendSeqNum++;
    return packedLength;
}
