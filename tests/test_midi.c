#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <midi.h>

static void test_midi_triggers_synth_note_on(void** state)
{
    expect_value(__wrap_synth_noteOn, channel, 0);
    expect_value(__wrap_synth_pitch, octave, 3);
    expect_value(__wrap_synth_pitch, freqNumber, 653);

    Message noteOn = { 0b10010000, 60, 127 };

    __real_midi_process(&noteOn);
}

static void test_midi_triggers_synth_note_off(void** state)
{
    expect_value(__wrap_synth_noteOff, channel, 0);

    Message noteOn = { 0b10000000, 0x40, 127 };

    __real_midi_process(&noteOn);
}

static void test_midi_sets_pitch(void** state)
{
    expect_value(__wrap_synth_noteOn, channel, 0);
    expect_value(__wrap_synth_pitch, octave, 3);
    expect_value(__wrap_synth_pitch, freqNumber, 653);

    const u16 MIDDLE_C = 60;

    Message noteOn = { 0b10010000, MIDDLE_C, 127 };

    __real_midi_process(&noteOn);
}

static void test_midi_sets_pitch_2(void** state)
{
    expect_value(__wrap_synth_noteOn, channel, 0);
    expect_value(__wrap_synth_pitch, octave, 6);
    expect_value(__wrap_synth_pitch, freqNumber, 1164);

    const u16 A_SHARP = 106;

    Message noteOn = { 0b10010000, A_SHARP, 127 };

    __real_midi_process(&noteOn);
}

static void test_process_does_nothing_for_channel_non_zero(void** state)
{
    for (int i = 1; i < 7; i++) {
        Message noteOn = { 0b10010000 + i, 106, 127 };

        __real_midi_process(&noteOn);
    }
}

static void test_process_does_nothing_for_control_change(void** state)
{
    Message noteOn = { 0xA0, 106, 127 };

    __real_midi_process(&noteOn);
}
