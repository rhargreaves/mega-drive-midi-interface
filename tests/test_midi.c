#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "midi.h"
#include <cmocka.h>

extern void __real_midi_noteOn(u8 chan, u8 pitch, u8 velocity);
extern void __real_midi_noteOff(u8 chan);
extern void __real_midi_channelVolume(u8 chan, u8 volume);
extern void __real_midi_pan(u8 chan, u8 pan);

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

        __real_midi_noteOff(chan);
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

static void test_midi_triggers_psg_note_on(void** state)
{
    const u8 midiKeys[] = { 36, 69, 108 };
    const u16 freqs[] = { 65, 440, 4186 };

    for (u8 i = 0; i < sizeof(midiKeys); i++) {
        for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {

            u16 expectedFrequency = freqs[i];
            u8 expectedMidiKey = midiKeys[i];

            expect_value(__wrap_psg_noteOn, channel, chan - MIN_PSG_CHAN);
            expect_value(__wrap_psg_noteOn, freq, expectedFrequency);
            expect_value(__wrap_psg_noteOn, attenuation, 0);

            __real_midi_noteOn(chan, expectedMidiKey, 127);
        }
    }
}

static void test_midi_triggers_psg_note_off(void** state)
{
    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        expect_value(__wrap_psg_noteOff, channel, chan - MIN_PSG_CHAN);

        __real_midi_noteOff(chan);
    }
}

static void test_midi_channel_volume_sets_total_level(void** state)
{
    expect_value(__wrap_synth_totalLevel, channel, 0);
    expect_value(__wrap_synth_totalLevel, totalLevel, 12);

    __real_midi_channelVolume(0, 60);
}

static void test_midi_channel_volume_sets_psg_attenuation(void** state)
{
    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_value(__wrap_psg_attenuation, attenuation, 2);

    __real_midi_channelVolume(MIN_PSG_CHAN, 60);
}

static void test_midi_pan_sets_synth_stereo_mode_right(void** state)
{
    expect_value(__wrap_synth_stereo, channel, 0);
    expect_value(__wrap_synth_stereo, mode, 1);

    __real_midi_pan(0, 127);
}

static void test_midi_pan_sets_synth_stereo_mode_left(void** state)
{
    expect_value(__wrap_synth_stereo, channel, 0);
    expect_value(__wrap_synth_stereo, mode, 2);

    __real_midi_pan(0, 0);
}

static void test_midi_pan_sets_synth_stereo_mode_centre(void** state)
{
    expect_value(__wrap_synth_stereo, channel, 0);
    expect_value(__wrap_synth_stereo, mode, 3);

    __real_midi_pan(0, 32);

    expect_value(__wrap_synth_stereo, channel, 0);
    expect_value(__wrap_synth_stereo, mode, 3);

    __real_midi_pan(0, 95);
}
