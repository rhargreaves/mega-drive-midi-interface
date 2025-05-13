
#include "midi_rx.h"
#include "comm/comm.h"
#include "midi.h"
#include "scheduler.h"
#include "synth.h"
#include "ui.h"
#include "comm/applemidi.h"
#include "log.h"
#include "settings.h"

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

typedef u8 (*Reader)(void);

static void read_message(Reader reader);
static void read_note_on(u8 status, Reader reader);
static void read_note_off(u8 status, Reader reader);
static void read_control_change(u8 status, Reader reader);
static void read_pitch_bend(u8 status, Reader reader);
static void read_program(u8 status, Reader reader);
static u16 read_14bit_value(Reader reader);
static void read_sysex(Reader reader);
static void read_system_message(u8 status, Reader reader);

static size_t startup_sequence_length = 0;
static size_t startup_sequence_index = 0;
static const u8* startup_sequence = NULL;

void midi_rx_init(void)
{
    scheduler_addTickHandler(midi_rx_read_if_comm_ready);
}

void midi_rx_read_if_comm_ready(void)
{
    while (comm_read_ready()) {
        midi_rx_read();
    }
}

void midi_rx_read(void)
{
    read_message(comm_read);
}

static u8 read_startup_sequence(void)
{
    const u8* startup_sequence = settings_startup_midi_sequence(&startup_sequence_length);
    if (startup_sequence_index >= startup_sequence_length) {
        SYS_die("Startup MIDI sequence index out of bounds");
    }
    return startup_sequence[startup_sequence_index++];
}

void midi_rx_run_startup_sequence(void)
{
    startup_sequence_index = 0;
    startup_sequence = settings_startup_midi_sequence(&startup_sequence_length);
    while (startup_sequence_index < startup_sequence_length) {
        read_message(read_startup_sequence);
    }
}

static void debug_print_event(u8 status, u8 data1, u8 data2)
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

static void read_message(Reader reader)
{
    u8 status = reader();
    u8 event = STATUS_UPPER(status);
    switch (event) {
    case EVENT_NOTE_ON:
        read_note_on(status, reader);
        break;
    case EVENT_NODE_OFF:
        read_note_off(status, reader);
        break;
    case EVENT_CC:
        read_control_change(status, reader);
        break;
    case EVENT_PITCH_BEND:
        read_pitch_bend(status, reader);
        break;
    case EVENT_PROGRAM:
        read_program(status, reader);
        break;
    case EVENT_SYSTEM:
        read_system_message(status, reader);
        break;
    default:
        log_warn("Status? %02X", status);
        break;
    }
}

static void read_control_change(u8 status, Reader reader)
{
    u8 chan = STATUS_LOWER(status);
    u8 controller = reader();
    u8 value = reader();
    debug_print_event(status, controller, value);
    midi_cc(chan, controller, value);
}

static void read_note_on(u8 status, Reader reader)
{
    u8 chan = STATUS_LOWER(status);
    u8 pitch = reader();
    u8 velocity = reader();
    debug_print_event(status, pitch, velocity);
    midi_note_on(chan, pitch, velocity);
}

static void read_note_off(u8 status, Reader reader)
{
    u8 chan = STATUS_LOWER(status);
    u8 pitch = reader();
    reader();
    debug_print_event(status, pitch, 0);
    midi_note_off(chan, pitch);
}

static void read_pitch_bend(u8 status, Reader reader)
{
    u8 chan = STATUS_LOWER(status);
    u16 bend = read_14bit_value(reader);
    debug_print_event(status, (u8)bend, 0);
    midi_pitch_bend(chan, bend);
}

static void read_program(u8 status, Reader reader)
{
    u8 chan = STATUS_LOWER(status);
    u8 program = reader();
    debug_print_event(status, program, 0);
    midi_program(chan, program);
}

static u16 read_14bit_value(Reader reader)
{
    u16 lower = reader();
    u16 upper = reader();
    return (upper << 7) + lower;
}

static void read_system_message(u8 status, Reader reader)
{
    u8 type = STATUS_LOWER(status);
    debug_print_event(status, 0, 0);
    switch (type) {
    case SYSTEM_SONG_POSITION:
        read_14bit_value(reader);
        break;
    case SYSTEM_CLOCK:
    case SYSTEM_START:
    case SYSTEM_CONTINUE:
    case SYSTEM_STOP:
        break;
    case SYSTEM_SYSEX:
        read_sysex(reader);
        break;
    case SYSTEM_RESET:
        log_warn("Reset all");
        midi_reset();
        midi_rx_run_startup_sequence();
        break;
    default:
        log_warn("System Status? %02X", status);
        break;
    }
}

static void read_sysex(Reader reader)
{
    const u16 BUFFER_LENGTH = 256;
    u8 buffer[BUFFER_LENGTH];
    u8 data;
    u16 index = 0;
    while (index < BUFFER_LENGTH && (data = reader()) != SYSEX_END) {
        buffer[index++] = data;
    }
    midi_sysex(buffer, index);
}
