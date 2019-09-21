#include "test_midi.h"

static void test_midi_increments_beat_every_24th_clock(UNUSED void** state)
{
    for (u16 i = 0; i < 24 * 2; i++) {
        __real_midi_clock();
    }

    assert_int_equal(__real_midi_timing()->barBeat, 2);
}

static void test_midi_increments_clocks(UNUSED void** state)
{
    for (u16 i = 0; i < 24 * 2; i++) {
        __real_midi_clock();
    }

    assert_int_equal(__real_midi_timing()->clocks, 48);
}

static void test_midi_start_resets_clock(UNUSED void** state)
{
    for (u16 i = 0; i < 24; i++) {
        __real_midi_clock();
    }
    assert_int_equal(__real_midi_timing()->clocks, 24);

    __real_midi_start();
    assert_int_equal(__real_midi_timing()->clocks, 0);
}

static void test_midi_position_sets_correct_timing(UNUSED void** state)
{
    const u16 bars = 2;
    const u16 quarterNotes = 1;
    const u16 sixteenths = 2;
    const u16 midiBeats = sixteenths + (quarterNotes * 4) + (bars * 16);
    const u16 clocks = midiBeats * 6;

    __real_midi_position(midiBeats);

    Timing* timing = __real_midi_timing();
    assert_int_equal(timing->bar, bars);
    assert_int_equal(timing->barBeat, quarterNotes);
    assert_int_equal(timing->sixteenth, sixteenths);
    assert_int_equal(timing->clocks, clocks);
    assert_int_equal(timing->clock, 0);
}

static void test_midi_timing_sets_bar_number(UNUSED void** state)
{
    __real_midi_position(0);
    for (u16 i = 0; i < (6 * 4 * 6) + 6; i++) {
        __real_midi_clock();
    };

    Timing* timing = __real_midi_timing();
    assert_int_equal(timing->bar, 1);
    assert_int_equal(timing->barBeat, 2);
    assert_int_equal(timing->sixteenth, 1);
}
