#include "test_midi_receiver.h"
#include "midi.h"
#include "midi_receiver.h"
#include "comm/comm.h"
#include "mocks/mock_comm.h"
#include "mocks/mock_midi.h"
#include "mocks/mock_log.h"

#define STATUS_CC 0xB0
#define STATUS_PITCH_BEND 0xE0
#define STATUS_CLOCK 0xF8
#define STATUS_STOP 0xFC
#define STATUS_START 0xFA
#define STATUS_CONTINUE 0xFB
#define STATUS_SONG_POSITION 0xF2
#define STATUS_PROGRAM 0xC0
#define STATUS_RESET 0xFF
#define STATUS_SYSEX_START 0xF0

void midi_receiver_read(void);

static void init(void)
{
    expect_any(__wrap_scheduler_addTickHandler, onTick);
    midi_receiver_init();
}

int test_midi_receiver_setup(UNUSED void** state)
{
    init();
    return 0;
}

void test_midi_receiver_initialises(UNUSED void** state)
{
    init();
}

void test_midi_receiver_read_passes_note_on_to_midi_processor(UNUSED void** state)
{
    const u8 expectedData = 60;
    const u8 expectedData2 = 127;

    for (int chan = 0; chan < MIDI_CHANNELS; chan++) {
        u8 expectedStatus = 0x90 + chan;
        expect_comm_read(COMM_OK, expectedStatus, COMM_DEFAULT_ATTEMPTS);
        expect_comm_read(COMM_OK, expectedData, COMM_DEFAULT_ATTEMPTS);
        expect_comm_read(COMM_OK, expectedData2, COMM_DEFAULT_ATTEMPTS);
        expect_midi_note_on(chan, expectedData, expectedData2);
        midi_receiver_read();
    }
}

void test_midi_receiver_read_passes_note_off_to_midi_processor(UNUSED void** state)
{
    u8 expectedStatus = 0x80;
    u8 expectedData = 60;
    u8 expectedData2 = 127;

    expect_comm_read(COMM_OK, expectedStatus, COMM_DEFAULT_ATTEMPTS);
    expect_comm_read(COMM_OK, expectedData, COMM_DEFAULT_ATTEMPTS);
    expect_comm_read(COMM_OK, expectedData2, COMM_DEFAULT_ATTEMPTS);
    expect_midi_note_off(0, expectedData);
    midi_receiver_read();
}

void test_midi_receiver_does_nothing_for_control_change(UNUSED void** state)
{
    u8 expectedStatus = 0xA0;
    u8 expectedData = 106;
    u8 expectedData2 = 127;

    expect_comm_read(COMM_OK, expectedStatus, COMM_DEFAULT_ATTEMPTS);
    expect_comm_read(COMM_OK, expectedData, COMM_DEFAULT_ATTEMPTS);
    expect_comm_read(COMM_OK, expectedData2, COMM_DEFAULT_ATTEMPTS);

    midi_receiver_read();
    midi_receiver_read();
    midi_receiver_read();
}

void test_midi_receiver_sets_unknown_event_for_unknown_status(UNUSED void** state)
{
    mock_log_enable_checks();

    u8 expectedStatus = 0xD0;

    expect_comm_read(COMM_OK, expectedStatus, COMM_DEFAULT_ATTEMPTS);
    expect_log_warn("Status? %02X");

    midi_receiver_read();
}

void test_midi_receiver_sets_unknown_event_for_unknown_system_message(UNUSED void** state)
{
    mock_log_enable_checks();

    u8 expectedStatus = 0xF1;

    expect_comm_read(COMM_OK, expectedStatus, COMM_DEFAULT_ATTEMPTS);
    expect_log_warn("System Status? %02X");

    midi_receiver_read();
}

void test_midi_receiver_sets_CC(UNUSED void** state)
{
    u8 expectedStatus = STATUS_CC;
    u8 expectedController = CC_VOLUME;
    u8 expectedValue = 0x80;

    expect_comm_read(COMM_OK, expectedStatus, COMM_DEFAULT_ATTEMPTS);
    expect_comm_read(COMM_OK, expectedController, COMM_DEFAULT_ATTEMPTS);
    expect_comm_read(COMM_OK, expectedValue, COMM_DEFAULT_ATTEMPTS);
    expect_midi_cc(0, expectedController, expectedValue);
    midi_receiver_read();
}

void test_midi_receiver_sets_pitch_bend(UNUSED void** state)
{
    u8 expectedStatus = STATUS_PITCH_BEND;
    u16 expectedValue = 12000;
    u8 expectedValueLower = expectedValue & 0x007F;
    u8 expectedValueUpper = expectedValue >> 7;

    expect_comm_read(COMM_OK, expectedStatus, COMM_DEFAULT_ATTEMPTS);
    expect_comm_read(COMM_OK, expectedValueLower, COMM_DEFAULT_ATTEMPTS);
    expect_comm_read(COMM_OK, expectedValueUpper, COMM_DEFAULT_ATTEMPTS);
    expect_midi_pitch_bend(0, expectedValue);
    midi_receiver_read();
}

void test_midi_receiver_does_nothing_on_midi_clock(UNUSED void** state)
{
    u8 status = STATUS_CLOCK;

    expect_comm_read(COMM_OK, status, COMM_DEFAULT_ATTEMPTS);

    midi_receiver_read();
}

void test_midi_receiver_does_nothing_on_midi_start_midi(UNUSED void** state)
{
    u8 status = STATUS_START;

    expect_comm_read(COMM_OK, status, COMM_DEFAULT_ATTEMPTS);

    midi_receiver_read();
}

void test_midi_receiver_swallows_midi_stop(UNUSED void** state)
{
    u8 status = STATUS_STOP;

    expect_comm_read(COMM_OK, status, COMM_DEFAULT_ATTEMPTS);

    midi_receiver_read();
}

void test_midi_receiver_swallows_midi_continue(UNUSED void** state)
{
    u8 status = STATUS_CONTINUE;

    expect_comm_read(COMM_OK, status, COMM_DEFAULT_ATTEMPTS);

    midi_receiver_read();
}

void test_midi_receiver_does_nothing_on_midi_position(UNUSED void** state)
{
    u8 status = STATUS_SONG_POSITION;
    u8 lower = 0x01;
    u8 upper = 0x02;

    expect_comm_read(COMM_OK, status, COMM_DEFAULT_ATTEMPTS);
    expect_comm_read(COMM_OK, lower, COMM_DEFAULT_ATTEMPTS);
    expect_comm_read(COMM_OK, upper, COMM_DEFAULT_ATTEMPTS);

    midi_receiver_read();
}

void test_midi_receiver_sets_midi_program(UNUSED void** state)
{
    u8 status = STATUS_PROGRAM;
    u8 program = 12;

    expect_comm_read(COMM_OK, status, COMM_DEFAULT_ATTEMPTS);
    expect_comm_read(COMM_OK, program, COMM_DEFAULT_ATTEMPTS);
    expect_midi_program(0, program);
    midi_receiver_read();
}

void test_midi_receiver_sends_midi_reset(UNUSED void** state)
{
    u8 status = STATUS_RESET;

    expect_comm_read(COMM_OK, status, COMM_DEFAULT_ATTEMPTS);
    expect_function_call(__wrap_midi_reset);
    expect_log_warn("Reset all");

    midi_receiver_read();
}

void test_midi_receiver_sends_sysex_to_midi_layer(UNUSED void** state)
{
    const u8 command = 0x12;
    expect_comm_read(COMM_OK, STATUS_SYSEX_START, COMM_DEFAULT_ATTEMPTS);
    expect_comm_read(COMM_OK, command, COMM_DEFAULT_ATTEMPTS);
    expect_comm_read(COMM_OK, SYSEX_END, COMM_DEFAULT_ATTEMPTS);

    u8 data[1] = { command };

    expect_memory(__wrap_midi_sysex, data, &data, 1);
    expect_value(__wrap_midi_sysex, length, 1);

    midi_receiver_read();
}

void test_midi_receiver_handles_sysex_limits(UNUSED void** state)
{
    const u16 SYSEX_BUFFER_SIZE = 256;
    const u16 SYSEX_MESSAGE_SIZE = 300;

    const u8 command = 0x12;
    expect_comm_read(COMM_OK, STATUS_SYSEX_START, COMM_DEFAULT_ATTEMPTS);
    for (u16 i = 0; i < SYSEX_MESSAGE_SIZE; i++) {
        expect_comm_read(COMM_OK, command, COMM_DEFAULT_ATTEMPTS);
    }
    expect_comm_read(COMM_OK, SYSEX_END, COMM_DEFAULT_ATTEMPTS);

    u8 data[SYSEX_BUFFER_SIZE];
    for (u16 i = 0; i < SYSEX_BUFFER_SIZE; i++) {
        data[i] = command;
    }

    expect_memory(__wrap_midi_sysex, data, &data, SYSEX_BUFFER_SIZE);
    expect_value(__wrap_midi_sysex, length, SYSEX_BUFFER_SIZE);

    midi_receiver_read();

    for (u16 i = 0; i < SYSEX_MESSAGE_SIZE - SYSEX_BUFFER_SIZE + 1; i++) {
        u8 dummy;
        comm_read(&dummy, COMM_DEFAULT_ATTEMPTS);
    }
}
