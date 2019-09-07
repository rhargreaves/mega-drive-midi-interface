#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "asserts.h"
#include "interface.h"
#include "midi.h"
#include "unused.h"
#include "wraps.h"
#include <cmocka.h>
#include <types.h>

#define STATUS_CC 0xB0
#define STATUS_PITCH_BEND 0xE0
#define STATUS_CLOCK 0xF8
#define STATUS_STOP 0xFC
#define STATUS_START 0xFA
#define STATUS_CONTINUE 0xFB
#define STATUS_SONG_POSITION 0xF2
#define STATUS_PROGRAM 0xC0
#define STATUS_SYSEX_START 0xF0
#define SYSEX_END 0xF7

static void test_interface_tick_passes_note_on_to_midi_processor(
    UNUSED void** state)
{
    const u8 expectedData = 60;
    const u8 expectedData2 = 127;

    for (int chan = 0; chan < MAX_MIDI_CHANS; chan++) {
        u8 expectedStatus = 0x90 + chan;

        stub_comm_read_returns_midi_event(
            expectedStatus, expectedData, expectedData2);

        expect_value(__wrap_midi_noteOn, chan, chan);
        expect_value(__wrap_midi_noteOn, pitch, expectedData);
        expect_value(__wrap_midi_noteOn, velocity, expectedData2);

        interface_tick();
    }
}

static void test_interface_tick_passes_note_off_to_midi_processor(
    UNUSED void** state)
{
    u8 expectedStatus = 0x80;
    u8 expectedData = 60;
    u8 expectedData2 = 127;

    stub_comm_read_returns_midi_event(
        expectedStatus, expectedData, expectedData2);

    expect_value(__wrap_midi_noteOff, chan, 0);
    expect_value(__wrap_midi_noteOff, pitch, expectedData);

    interface_tick();
}

static void test_interface_does_nothing_for_control_change(UNUSED void** state)
{
    u8 expectedStatus = 0xA0;
    u8 expectedData = 106;
    u8 expectedData2 = 127;

    stub_comm_read_returns_midi_event(
        expectedStatus, expectedData, expectedData2);

    interface_tick();
    interface_tick();
    interface_tick();
}

static void test_interface_sets_unknown_event_for_unknown_status(
    UNUSED void** state)
{
    u8 expectedStatus = 0xD0;

    will_return(__wrap_comm_read, expectedStatus);

    interface_tick();

    assert_int_equal(interface_lastUnknownStatus(), expectedStatus);
}

static void test_interface_sets_unknown_event_for_unknown_system_message(
    UNUSED void** state)
{
    u8 expectedStatus = 0xF1;

    will_return(__wrap_comm_read, expectedStatus);

    interface_tick();

    assert_int_equal(interface_lastUnknownStatus(), expectedStatus);
}

static void test_interface_sets_CC(UNUSED void** state)
{
    u8 expectedStatus = STATUS_CC;
    u8 expectedController = CC_VOLUME;
    u8 expectedValue = 0x80;

    stub_comm_read_returns_midi_event(
        expectedStatus, expectedController, expectedValue);

    expect_value(__wrap_midi_cc, chan, 0);
    expect_value(__wrap_midi_cc, controller, expectedController);
    expect_value(__wrap_midi_cc, value, expectedValue);

    interface_tick();

    ControlChange* cc = midi_lastUnknownCC();
    assert_int_not_equal(cc->controller, expectedController);
    assert_int_not_equal(cc->value, expectedValue);
}

static void test_interface_initialises_synth(UNUSED void** state)
{
    expect_function_call(__wrap_synth_init);
    interface_init();
}

static void test_interface_sets_pitch_bend(UNUSED void** state)
{
    u8 expectedStatus = STATUS_PITCH_BEND;
    u16 expectedValue = 12000;
    u8 expectedValueLower = expectedValue & 0x007F;
    u8 expectedValueUpper = expectedValue >> 7;

    stub_comm_read_returns_midi_event(
        expectedStatus, expectedValueLower, expectedValueUpper);

    expect_value(__wrap_midi_pitchBend, chan, 0);
    expect_value(__wrap_midi_pitchBend, bend, expectedValue);

    interface_tick();
}

static void test_interface_increments_midi_clock(UNUSED void** state)
{
    u8 status = STATUS_CLOCK;
    will_return(__wrap_comm_read, status);

    expect_function_call(__wrap_midi_clock);

    interface_tick();
}

static void test_interface_starts_midi(UNUSED void** state)
{
    u8 status = STATUS_START;
    will_return(__wrap_comm_read, status);

    expect_function_call(__wrap_midi_start);

    interface_tick();
}

static void test_interface_swallows_midi_stop(UNUSED void** state)
{
    interface_reset();

    u8 status = STATUS_STOP;
    will_return(__wrap_comm_read, status);

    interface_tick();

    assert_int_equal(interface_lastUnknownStatus(), 0);
}

static void test_interface_swallows_midi_continue(UNUSED void** state)
{
    interface_reset();

    u8 status = STATUS_CONTINUE;
    will_return(__wrap_comm_read, status);

    interface_tick();

    assert_int_equal(interface_lastUnknownStatus(), 0);
}

static void test_interface_sets_position(UNUSED void** state)
{
    u8 status = STATUS_SONG_POSITION;
    u16 beat = 0x3FFF;
    u8 beatLSB = beat & 0x007F;
    u8 beatMSB = (beat >> 7) & 0x007F;

    will_return(__wrap_comm_read, status);
    will_return(__wrap_comm_read, beatLSB);
    will_return(__wrap_comm_read, beatMSB);

    expect_value(__wrap_midi_position, beat, beat);

    interface_tick();
}

static void test_interface_sets_midi_program(UNUSED void** state)
{
    u8 status = STATUS_PROGRAM;
    u8 program = 12;

    will_return(__wrap_comm_read, status);
    will_return(__wrap_comm_read, program);

    expect_value(__wrap_midi_program, chan, 0);
    expect_value(__wrap_midi_program, program, program);

    interface_tick();
}


static void test_interface_sends_sysex_to_midi_layer(UNUSED void** state)
{
    const u8 command = 0x12;
    will_return(__wrap_comm_read, STATUS_SYSEX_START);
    will_return(__wrap_comm_read, command);
    will_return(__wrap_comm_read, SYSEX_END);

    u8 data[1] = { command };

    expect_memory(__wrap_midi_sysex, data, &data, 1);
    expect_value(__wrap_midi_sysex, length, 1);

    interface_tick();
}
