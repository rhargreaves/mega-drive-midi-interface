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

    __real_midi_noteOn(0, 60, 127);
}

static void test_midi_triggers_synth_note_off(void** state)
{
    expect_value(__wrap_synth_noteOff, channel, 0);

    __real_midi_noteOff(0, 60, 127);
}

static void test_midi_triggers_synth_note_on_2(void** state)
{
    expect_value(__wrap_synth_noteOn, channel, 0);
    expect_value(__wrap_synth_pitch, octave, 6);
    expect_value(__wrap_synth_pitch, freqNumber, 1164);

    const u16 A_SHARP = 106;

    __real_midi_noteOn(0, A_SHARP, 127);
}
