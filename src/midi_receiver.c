#include "midi_receiver.h"
#include "comm/comm.h"
#include "midi.h"
#include "scheduler.h"
#include "synth.h"
#include "ui.h"
#include "comm/applemidi.h"
#include "log.h"

#define STATUS_LOWER(status) (status & 0x0F)
#define STATUS_UPPER(status) (status >> 4)

#define EVENT_PITCH_BEND 0xE
#define EVENT_NOTE_ON 0x9
#define EVENT_NODE_OFF 0x8
#define EVENT_CC 0xB
#define EVENT_PROGRAM 0xC
#define EVENT_SYSTEM 0xF

#define SYSTEM_CLOCK 0x8
#define SYSTEM_START 0xA
#define SYSTEM_STOP 0xC
#define SYSTEM_CONTINUE 0xB
#define SYSTEM_SONG_POSITION 0x2
#define SYSTEM_SYSEX 0x0
#define SYSTEM_RESET 0xF

static void noteOn(u8 status);
static void noteOff(u8 status);
static void controlChange(u8 status);
static void pitchBend(u8 status);
static void systemMessage(u8 status);
static void program(u8 status);
static u16 read14bitValue(void);
static void readSysEx(void);
static void processStatus(u8 status);

void midi_receiver_init(void)
{
    scheduler_addTickHandler(midi_receiver_read_if_comm_ready);
}

void midi_receiver_read_if_comm_ready(void)
{
    u8 data;
    while (comm_read_ready()) {
        if (comm_read(&data, 0) == COMM_OK) {
            processStatus(data);
        } else {
            break;
        }
    }
}

static void debugPrintEvent(u8 status, u8 data1, u8 data2)
{
#if DEBUG_EVENTS
    u16 seqNum = applemidi_lastSequenceNumber();
    char t[30];
    sprintf(t, "S:%02X D1:%02X D2:%02X Sn:%05u", status, data1, data2, seqNum);
    ui_draw_text(t, 7, 1);
#else
    (void)status;
    (void)data1;
    (void)data2;
#endif
}

void midi_receiver_read(void)
{
    u8 status;
    if (comm_read(&status, COMM_DEFAULT_ATTEMPTS) != COMM_OK) {
        return;
    }

    processStatus(status);
}

static void processStatus(u8 status)
{
    u8 event = STATUS_UPPER(status);
    switch (event) {
    case EVENT_NOTE_ON:
        noteOn(status);
        break;
    case EVENT_NODE_OFF:
        noteOff(status);
        break;
    case EVENT_CC:
        controlChange(status);
        break;
    case EVENT_PITCH_BEND:
        pitchBend(status);
        break;
    case EVENT_PROGRAM:
        program(status);
        break;
    case EVENT_SYSTEM:
        systemMessage(status);
        break;
    default:
        log_warn("Status? %02X", status);
        break;
    }
}

static void controlChange(u8 status)
{
    u8 chan = STATUS_LOWER(status);
    u8 controller, value;

    if (comm_read(&controller, COMM_DEFAULT_ATTEMPTS) != COMM_OK) {
        log_warn("MIDIRecv: Failed to read CC");
        return;
    }

    if (comm_read(&value, COMM_DEFAULT_ATTEMPTS) != COMM_OK) {
        log_warn("MIDIRecv: Failed to read CC value");
        return;
    }

    debugPrintEvent(status, controller, value);
    midi_cc(chan, controller, value);
}

static void noteOn(u8 status)
{
    u8 chan = STATUS_LOWER(status);
    u8 pitch, velocity;

    if (comm_read(&pitch, COMM_DEFAULT_ATTEMPTS) != COMM_OK) {
        log_warn("MIDIRecv: Failed to read note on pitch");
        return;
    }

    if (comm_read(&velocity, COMM_DEFAULT_ATTEMPTS) != COMM_OK) {
        log_warn("MIDIRecv: Failed to read note on velocity");
        return;
    }

    debugPrintEvent(status, pitch, velocity);
    midi_note_on(chan, pitch, velocity);
}

static void noteOff(u8 status)
{
    u8 chan = STATUS_LOWER(status);
    u8 pitch, velocity;

    if (comm_read(&pitch, COMM_DEFAULT_ATTEMPTS) != COMM_OK) {
        log_warn("MIDIRecv: Failed to read note off pitch");
        return;
    }

    if (comm_read(&velocity, COMM_DEFAULT_ATTEMPTS) != COMM_OK) {
        log_warn("MIDIRecv: Failed to read note off velocity");
        return;
    }

    debugPrintEvent(status, pitch, 0);
    midi_note_off(chan, pitch);
}

static void pitchBend(u8 status)
{
    u8 chan = STATUS_LOWER(status);
    u16 bend = read14bitValue();
    debugPrintEvent(status, (u8)bend, 0);
    midi_pitch_bend(chan, bend);
}

static void program(u8 status)
{
    u8 chan = STATUS_LOWER(status);
    u8 program;

    if (comm_read(&program, COMM_DEFAULT_ATTEMPTS) != COMM_OK) {
        log_warn("MIDIRecv: Failed to read program");
        return;
    }

    debugPrintEvent(status, program, 0);
    midi_program(chan, program);
}

static u16 read14bitValue(void)
{
    u8 lower, upper;

    if (comm_read(&lower, COMM_DEFAULT_ATTEMPTS) != COMM_OK) {
        log_warn("MIDIRecv: Failed to read 14bit value lower");
        return 0;
    }

    if (comm_read(&upper, COMM_DEFAULT_ATTEMPTS) != COMM_OK) {
        log_warn("MIDIRecv: Failed to read 14bit value upper");
        return 0;
    }

    return (upper << 7) + lower;
}

static void systemMessage(u8 status)
{
    u8 type = STATUS_LOWER(status);
    debugPrintEvent(status, 0, 0);
    switch (type) {
    case SYSTEM_SONG_POSITION:
        read14bitValue();
        break;
    case SYSTEM_CLOCK:
    case SYSTEM_START:
    case SYSTEM_CONTINUE:
    case SYSTEM_STOP:
        break;
    case SYSTEM_SYSEX:
        readSysEx();
        break;
    case SYSTEM_RESET:
        log_warn("Reset all");
        midi_reset();
        break;
    default:
        log_warn("System Status? %02X", status);
        break;
    }
}

static void readSysEx(void)
{
    const u16 BUFFER_LENGTH = 256;
    u8 buffer[BUFFER_LENGTH];
    u8 data;
    u16 index = 0;

    while (index < BUFFER_LENGTH) {
        if (comm_read(&data, COMM_DEFAULT_ATTEMPTS) != COMM_OK) {
            log_warn("MIDIRecv: Failed to read sysex");
            break;
        }

        if (data == SYSEX_END) {
            break;
        }

        buffer[index++] = data;
    }

    midi_sysex(buffer, index);
}
