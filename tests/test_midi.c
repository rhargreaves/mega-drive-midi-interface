#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <midi.h>

static void test_midi_triggers_synth_note_on(void** state)
{
    for (int chan = 0; chan < MAX_MIDI_CHANS; chan++) {
        expect_value(__wrap_synth_pitch, channel, chan);
        expect_value(__wrap_synth_pitch, octave, 3);
        expect_value(__wrap_synth_pitch, freqNumber, 653);
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_noteOn(chan, 60, 127);
    }
}

static void test_midi_triggers_synth_note_off(void** state)
{
    for (int chan = 0; chan < MAX_MIDI_CHANS; chan++) {
        expect_value(__wrap_synth_noteOff, channel, chan);

        __real_midi_noteOff(chan, 60, 127);
    }
}

static void test_midi_triggers_synth_note_on_2(void** state)
{
    expect_value(__wrap_synth_pitch, channel, 0);
    expect_value(__wrap_synth_pitch, octave, 6);
    expect_value(__wrap_synth_pitch, freqNumber, 1164);
    expect_value(__wrap_synth_noteOn, channel, 0);

    const u16 A_SHARP = 106;

    __real_midi_noteOn(0, A_SHARP, 127);
}
