#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "asserts.h"
#include "constants.h"
#include "midi.h"
#include "midi_psg.h"
#include "psg_chip.h"
#include "synth.h"
#include "unused.h"
#include <cmocka.h>

extern void __real_midi_noteOn(u8 chan, u8 pitch, u8 velocity);
extern void __real_midi_noteOff(u8 chan, u8 pitch);
extern void __real_midi_pitchBend(u8 chan, u16 bend);
extern bool __real_midi_getPolyphonic(void);
extern void __real_midi_cc(u8 chan, u8 controller, u8 value);
extern void __real_midi_clock(void);
extern void __real_midi_start(void);
extern void __real_midi_position(u16 beat);
extern void __real_midi_program(u8 chan, u8 program);
extern Timing* __real_midi_timing(void);
extern void __real_midi_sysex(u8* data, u16 length);

static int test_midi_setup(UNUSED void** state)
{
    midi_init();
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch_any();

        __real_midi_pitchBend(chan, 0);
    }
    return 0;
}

static void test_midi_triggers_psg_note_on(UNUSED void** state)
{
    const u8 midiKeys[] = { 45, 69, 108 };
    const u16 freqs[] = { 110, 440, 4186 };

    for (u8 i = 0; i < sizeof(midiKeys); i++) {
        for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {

            u16 expectedFrequency = freqs[i];
            u8 expectedMidiKey = midiKeys[i];
            u8 expectedPsgChan = chan - MIN_PSG_CHAN;

            expect_value(__wrap_psg_frequency, channel, expectedPsgChan);
            expect_value(__wrap_psg_frequency, freq, expectedFrequency);
            expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
            expect_value(__wrap_psg_attenuation, attenuation, 0);

            __real_midi_noteOn(chan, expectedMidiKey, 127);
        }
    }
}

static void test_midi_triggers_psg_note_on_with_velocity(UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = chan - MIN_PSG_CHAN;

    expect_any(__wrap_psg_frequency, channel);
    expect_any(__wrap_psg_frequency, freq);
    expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
    expect_value(__wrap_psg_attenuation, attenuation, 0x3);

    __real_midi_noteOn(chan, 60, MAX_MIDI_VOLUME / 2);
}

static void test_midi_triggers_psg_note_on_with_velocity_and_channel_volume(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = chan - MIN_PSG_CHAN;

    __real_midi_cc(chan, CC_VOLUME, MAX_MIDI_VOLUME / 2);

    expect_any(__wrap_psg_frequency, channel);
    expect_any(__wrap_psg_frequency, freq);
    expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
    expect_value(__wrap_psg_attenuation, attenuation, 0x7);

    __real_midi_noteOn(chan, 60, MAX_MIDI_VOLUME / 2);
}

static void test_midi_changing_volume_during_psg_note_on_respects_velocity(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = chan - MIN_PSG_CHAN;

    expect_any(__wrap_psg_frequency, channel);
    expect_any(__wrap_psg_frequency, freq);
    expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
    expect_value(__wrap_psg_attenuation, attenuation, 0x3);

    __real_midi_noteOn(chan, 60, MAX_MIDI_VOLUME / 2);

    expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
    expect_value(__wrap_psg_attenuation, attenuation, 0x7);

    __real_midi_cc(chan, CC_VOLUME, MAX_MIDI_VOLUME / 2);
}

static void test_midi_triggers_psg_note_off(UNUSED void** state)
{
    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        u8 expectedPsgChan = chan - MIN_PSG_CHAN;
        u8 midiKey = 60;

        expect_any(__wrap_psg_frequency, channel);
        expect_any(__wrap_psg_frequency, freq);
        expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
        expect_value(
            __wrap_psg_attenuation, attenuation, PSG_ATTENUATION_LOUDEST);

        __real_midi_noteOn(chan, midiKey, 127);

        expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
        expect_value(
            __wrap_psg_attenuation, attenuation, PSG_ATTENUATION_SILENCE);

        __real_midi_noteOff(chan, midiKey);
    }
}

static void test_midi_drops_psg_key_below_45(UNUSED void** state)
{
    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {

        u8 expectedMidiKey = 44;
        __real_midi_noteOn(chan, expectedMidiKey, 127);
    }
}

static void
test_midi_triggers_psg_note_off_and_volume_change_does_not_cause_psg_channel_to_play(
    UNUSED void** state)
{
    u8 midiKey = 60;
    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        u8 expectedPsgChan = chan - MIN_PSG_CHAN;

        expect_any(__wrap_psg_frequency, channel);
        expect_any(__wrap_psg_frequency, freq);
        expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
        expect_value(
            __wrap_psg_attenuation, attenuation, PSG_ATTENUATION_LOUDEST);

        __real_midi_noteOn(chan, midiKey, 127);

        expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
        expect_value(
            __wrap_psg_attenuation, attenuation, PSG_ATTENUATION_SILENCE);

        __real_midi_noteOff(chan, midiKey);

        __real_midi_cc(MIN_PSG_CHAN, CC_VOLUME, 96);
    }
}

static void test_midi_psg_note_off_only_triggered_if_specific_note_is_on(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;

    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_value(__wrap_psg_attenuation, attenuation, PSG_ATTENUATION_LOUDEST);
    expect_value(__wrap_psg_frequency, channel, 0);
    expect_value(__wrap_psg_frequency, freq, 0x106);

    __real_midi_noteOn(chan, 60, 127);

    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_value(__wrap_psg_attenuation, attenuation, PSG_ATTENUATION_LOUDEST);
    expect_value(__wrap_psg_frequency, channel, 0);
    expect_value(__wrap_psg_frequency, freq, 0x115);

    __real_midi_noteOn(chan, 61, 127);

    __real_midi_noteOff(chan, 60);
}

static void test_midi_channel_volume_sets_psg_attenuation(UNUSED void** state)
{
    __real_midi_cc(MIN_PSG_CHAN, CC_VOLUME, 96);

    expect_value(__wrap_psg_frequency, channel, 0);
    expect_value(__wrap_psg_frequency, freq, 110);
    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_value(__wrap_psg_attenuation, attenuation, 1);

    __real_midi_noteOn(MIN_PSG_CHAN, 45, 127);
}

static void test_midi_channel_volume_sets_psg_attenuation_2(UNUSED void** state)
{
    __real_midi_cc(MIN_PSG_CHAN, CC_VOLUME, 127);

    expect_value(__wrap_psg_frequency, channel, 0);
    expect_value(__wrap_psg_frequency, freq, 110);
    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_value(__wrap_psg_attenuation, attenuation, 0);

    __real_midi_noteOn(MIN_PSG_CHAN, 45, 127);
}

static void test_midi_directs_channels_above_10_to_psg(UNUSED void** state)
{
    for (int chan = 11; chan < 16; chan++) {
        expect_any(__wrap_psg_frequency, channel);
        expect_any(__wrap_psg_frequency, freq);
        expect_any(__wrap_psg_attenuation, channel);
        expect_any(__wrap_psg_attenuation, attenuation);

        __real_midi_noteOn(chan, 60, 127);
    }
}

static void test_midi_sets_synth_pitch_bend(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, 653);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);
        __real_midi_noteOn(chan, 60, 127);

        expect_synth_pitch(chan, 4, 582);
        __real_midi_pitchBend(chan, 1000);
    }
}

static void test_midi_sets_psg_pitch_bend(UNUSED void** state)
{
    for (int chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {

        u8 expectedPsgChan = chan - MIN_PSG_CHAN;

        expect_value(__wrap_psg_frequency, channel, expectedPsgChan);
        expect_value(__wrap_psg_frequency, freq, 262);
        expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
        expect_value(__wrap_psg_attenuation, attenuation, 0);

        __real_midi_noteOn(chan, 60, 127);

        expect_value(__wrap_psg_frequency, channel, expectedPsgChan);
        expect_value(__wrap_psg_frequency, freq, 191);

        __real_midi_pitchBend(chan, 1000);
    }
}

static void test_midi_polyphonic_mode_returns_state(UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 127);

    assert_true(__real_midi_getPolyphonic());

    __real_midi_cc(0, CC_POLYPHONIC_MODE, 0);
}

static void test_midi_polyphonic_mode_uses_multiple_fm_channels(
    UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 127);

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(0, 6, 1164);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, 0);

        __real_midi_noteOn(chan, A_SHARP, 127);

        expect_synth_pitch(1, 7, 0x269);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, 1);

        __real_midi_noteOn(chan, B, 127);

        expect_value(__wrap_synth_noteOff, channel, 0);

        __real_midi_noteOff(chan, A_SHARP);

        expect_value(__wrap_synth_noteOff, channel, 1);

        __real_midi_noteOff(chan, B);
    }

    __real_midi_cc(0, CC_POLYPHONIC_MODE, 0);
}

static void test_midi_polyphonic_mode_note_off_silences_all_matching_pitch(
    UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 127);

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(0, 6, 1164);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, 0);

        __real_midi_noteOn(chan, A_SHARP, 127);

        expect_synth_pitch(1, 6, 1164);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, 1);

        __real_midi_noteOn(chan, A_SHARP, 127);

        expect_value(__wrap_synth_noteOff, channel, 0);
        expect_value(__wrap_synth_noteOff, channel, 1);

        __real_midi_noteOff(chan, A_SHARP);
    }

    __real_midi_cc(0, CC_POLYPHONIC_MODE, 0);
}

static void test_midi_sets_all_notes_off(UNUSED void** state)
{
    expect_value(__wrap_synth_noteOff, channel, 0);

    __real_midi_cc(0, CC_ALL_NOTES_OFF, 0);
}

static void test_midi_sets_all_notes_off_in_polyphonic_mode(UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 64);

    expect_value(__wrap_synth_pitch, channel, 0);
    expect_any(__wrap_synth_pitch, octave);
    expect_any(__wrap_synth_pitch, freqNumber);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);

    __real_midi_noteOn(0, A_SHARP, 127);

    expect_value(__wrap_synth_pitch, channel, 1);
    expect_any(__wrap_synth_pitch, octave);
    expect_any(__wrap_synth_pitch, freqNumber);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 1);

    __real_midi_noteOn(0, B, 127);

    for (u8 c = 0; c <= MAX_FM_CHAN; c++) {
        expect_value(__wrap_synth_noteOff, channel, c);
    }

    __real_midi_cc(0, CC_ALL_NOTES_OFF, 0);

    __real_midi_cc(0, CC_POLYPHONIC_MODE, 0);
}

static void test_midi_sets_polyphonic_mode(UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 64);

    assert_true(__real_midi_getPolyphonic());
}

static void test_midi_unsets_polyphonic_mode(UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 0);

    assert_false(__real_midi_getPolyphonic());
}

static void test_midi_sets_unknown_CC(UNUSED void** state)
{
    u8 expectedController = 0x9;
    u8 expectedValue = 0x50;

    __real_midi_cc(0, expectedController, expectedValue);

    ControlChange* cc = midi_lastUnknownCC();

    assert_int_equal(cc->controller, expectedController);
    assert_int_equal(cc->value, expectedValue);
}

static void test_midi_polyphonic_mode_sends_CCs_to_all_FM_channels(
    UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 127);

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_value(__wrap_synth_algorithm, channel, chan);
        expect_value(__wrap_synth_algorithm, algorithm, 1);
    }

    __real_midi_cc(0, CC_GENMDM_FM_ALGORITHM, 16);
}

static void test_midi_set_overflow_flag_on_polyphony_breach(UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 127);

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 6, 1164);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_noteOn(0, A_SHARP, 127);
    }

    __real_midi_noteOn(0, A_SHARP, 127);

    assert_true(midi_overflow());
}

static void test_midi_clears_overflow_flag(UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 127);

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_value(__wrap_synth_pitch, channel, chan);
        expect_any(__wrap_synth_pitch, octave);
        expect_any(__wrap_synth_pitch, freqNumber);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_noteOn(0, A_SHARP + chan, 127);
    }

    __real_midi_noteOn(0, A_SHARP + MAX_FM_CHAN + 1, 127);

    assert_true(midi_overflow());

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_value(__wrap_synth_noteOff, channel, chan);

        __real_midi_noteOff(0, A_SHARP + chan);
    }

    expect_value(__wrap_synth_pitch, channel, 0);
    expect_any(__wrap_synth_pitch, octave);
    expect_any(__wrap_synth_pitch, freqNumber);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);

    __real_midi_noteOn(0, A_SHARP, 127);

    assert_false(midi_overflow());
}
