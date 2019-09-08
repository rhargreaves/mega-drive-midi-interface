#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "midi.h"
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

static const u16 A_SHARP = 94;
static const u16 B = 95;

static int test_midi_setup(UNUSED void** state)
{
    midi_reset();
    midi_psg_reset();

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_any(__wrap_synth_pitch, channel);
        expect_any(__wrap_synth_pitch, octave);
        expect_any(__wrap_synth_pitch, freqNumber);

        __real_midi_pitchBend(chan, 0);
    }
    return 0;
}

static void test_midi_triggers_synth_note_on(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_value(__wrap_synth_pitch, channel, chan);
        expect_value(__wrap_synth_pitch, octave, 4);
        expect_value(__wrap_synth_pitch, freqNumber, 653);
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_noteOn(chan, 60, 127);
    }
}

static void test_midi_triggers_synth_note_on_boundary_values(
    UNUSED void** state)
{
    const u8 keys[] = { 11, 106 };
    const u16 expectedFrequencies[] = { 617, 1164 };
    const u8 expectedOctaves[] = { 0, 7 };

    for (int index = 0; index < 2; index++) {
        for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
            expect_value(__wrap_synth_pitch, channel, chan);
            expect_value(__wrap_synth_pitch, octave, expectedOctaves[index]);
            expect_value(
                __wrap_synth_pitch, freqNumber, expectedFrequencies[index]);
            expect_value(__wrap_synth_noteOn, channel, chan);

            __real_midi_noteOn(chan, keys[index], 127);
        }
    }
}

static void test_midi_does_not_trigger_synth_note_on_out_of_bound_values(
    UNUSED void** state)
{
    const u8 keys[] = { 0, 10, 107, 127 };

    for (int index = 0; index < 2; index++) {
        for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
            __real_midi_noteOn(chan, keys[index], 127);
        }
    }
}

static void test_midi_triggers_synth_note_off(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_value(__wrap_synth_noteOff, channel, chan);

        __real_midi_noteOff(chan, 0);
    }
}

static void test_midi_triggers_synth_note_on_2(UNUSED void** state)
{
    expect_value(__wrap_synth_pitch, channel, 0);
    expect_value(__wrap_synth_pitch, octave, 6);
    expect_value(__wrap_synth_pitch, freqNumber, 1164);
    expect_value(__wrap_synth_noteOn, channel, 0);

    __real_midi_noteOn(0, A_SHARP, 127);
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

static void test_midi_triggers_psg_note_off(UNUSED void** state)
{
    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        u8 expectedPsgChan = chan - MIN_PSG_CHAN;
        expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
        expect_value(
            __wrap_psg_attenuation, attenuation, PSG_ATTENUATION_SILENCE);

        __real_midi_noteOff(chan, 0);
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
    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        u8 expectedPsgChan = chan - MIN_PSG_CHAN;
        expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
        expect_value(
            __wrap_psg_attenuation, attenuation, PSG_ATTENUATION_SILENCE);

        __real_midi_noteOff(chan, 0);

        __real_midi_cc(MIN_PSG_CHAN, CC_VOLUME, 96);
    }
}

static void test_midi_channel_volume_sets_volume(UNUSED void** state)
{
    expect_value(__wrap_synth_volume, channel, 0);
    expect_value(__wrap_synth_volume, volume, 60);

    __real_midi_cc(0, CC_VOLUME, 60);
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

static void test_midi_pan_sets_synth_stereo_mode_right(UNUSED void** state)
{
    expect_value(__wrap_synth_stereo, channel, 0);
    expect_value(__wrap_synth_stereo, mode, 1);

    __real_midi_cc(0, CC_PAN, 127);
}

static void test_midi_pan_sets_synth_stereo_mode_left(UNUSED void** state)
{
    expect_value(__wrap_synth_stereo, channel, 0);
    expect_value(__wrap_synth_stereo, mode, 2);

    __real_midi_cc(0, CC_PAN, 0);
}

static void test_midi_pan_sets_synth_stereo_mode_centre(UNUSED void** state)
{
    expect_value(__wrap_synth_stereo, channel, 0);
    expect_value(__wrap_synth_stereo, mode, 3);

    __real_midi_cc(0, CC_PAN, 32);

    expect_value(__wrap_synth_stereo, channel, 0);
    expect_value(__wrap_synth_stereo, mode, 3);

    __real_midi_cc(0, CC_PAN, 95);
}

static void test_midi_ignores_channels_above_10(UNUSED void** state)
{
    for (int chan = 11; chan < 16; chan++) {
        __real_midi_noteOn(chan, 60, 127);
    }
}

static void test_midi_sets_synth_pitch_bend(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_value(__wrap_synth_pitch, channel, chan);
        expect_value(__wrap_synth_pitch, octave, 4);
        expect_value(__wrap_synth_pitch, freqNumber, 653);
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_noteOn(chan, 60, 127);

        expect_value(__wrap_synth_pitch, channel, chan);
        expect_value(__wrap_synth_pitch, octave, 4);
        expect_value(__wrap_synth_pitch, freqNumber, 582);

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
        expect_value(__wrap_synth_pitch, channel, 0);
        expect_value(__wrap_synth_pitch, octave, 6);
        expect_value(__wrap_synth_pitch, freqNumber, 1164);
        expect_value(__wrap_synth_noteOn, channel, 0);

        __real_midi_noteOn(chan, A_SHARP, 127);

        expect_value(__wrap_synth_pitch, channel, 1);
        expect_value(__wrap_synth_pitch, octave, 7);
        expect_value(__wrap_synth_pitch, freqNumber, 0x269);
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
        expect_value(__wrap_synth_pitch, channel, 0);
        expect_value(__wrap_synth_pitch, octave, 6);
        expect_value(__wrap_synth_pitch, freqNumber, 1164);
        expect_value(__wrap_synth_noteOn, channel, 0);

        __real_midi_noteOn(chan, A_SHARP, 127);

        expect_value(__wrap_synth_pitch, channel, 1);
        expect_value(__wrap_synth_pitch, octave, 6);
        expect_value(__wrap_synth_pitch, freqNumber, 1164);
        expect_value(__wrap_synth_noteOn, channel, 1);

        __real_midi_noteOn(chan, A_SHARP, 127);

        expect_value(__wrap_synth_noteOff, channel, 0);
        expect_value(__wrap_synth_noteOff, channel, 1);

        __real_midi_noteOff(chan, A_SHARP);
    }

    __real_midi_cc(0, CC_POLYPHONIC_MODE, 0);
}

static void test_midi_sets_fm_algorithm(UNUSED void** state)
{
    expect_value(__wrap_synth_algorithm, channel, 0);
    expect_value(__wrap_synth_algorithm, algorithm, 1);

    __real_midi_cc(0, CC_GENMDM_FM_ALGORITHM, 20);
}

static void test_midi_sets_fm_feedback(UNUSED void** state)
{
    expect_value(__wrap_synth_feedback, channel, 0);
    expect_value(__wrap_synth_feedback, feedback, 2);

    __real_midi_cc(0, CC_GENMDM_FM_FEEDBACK, 33);
}

static void test_midi_sets_channel_AMS(UNUSED void** state)
{
    expect_value(__wrap_synth_ams, channel, 0);
    expect_value(__wrap_synth_ams, ams, 1);

    __real_midi_cc(0, 76, 32);
}

static void test_midi_sets_channel_FMS(UNUSED void** state)
{
    expect_value(__wrap_synth_fms, channel, 0);
    expect_value(__wrap_synth_fms, fms, 2);

    __real_midi_cc(0, 75, 32);
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
    expect_value(__wrap_synth_noteOn, channel, 0);

    __real_midi_noteOn(0, A_SHARP, 127);

    expect_value(__wrap_synth_pitch, channel, 1);
    expect_any(__wrap_synth_pitch, octave);
    expect_any(__wrap_synth_pitch, freqNumber);
    expect_value(__wrap_synth_noteOn, channel, 1);

    __real_midi_noteOn(0, B, 127);

    for (u8 c = 0; c <= MAX_FM_CHAN; c++) {
        expect_value(__wrap_synth_noteOff, channel, c);
    }

    __real_midi_cc(0, CC_ALL_NOTES_OFF, 0);

    __real_midi_cc(0, CC_POLYPHONIC_MODE, 0);
}

static void test_midi_sets_operator_total_level(UNUSED void** state)
{
    const u8 expectedValue = 50;
    for (u8 cc = 16; cc <= 19; cc++) {
        u8 expectedOp = cc - 16;
        expect_value(__wrap_synth_operatorTotalLevel, channel, 0);
        expect_value(__wrap_synth_operatorTotalLevel, op, expectedOp);
        expect_value(
            __wrap_synth_operatorTotalLevel, totalLevel, expectedValue);

        __real_midi_cc(0, cc, expectedValue);
    }
}

static void test_midi_sets_operator_multiple(UNUSED void** state)
{
    const u8 expectedValue = 4;

    for (u8 cc = 20; cc <= 23; cc++) {
        u8 expectedOp = cc - 20;
        expect_value(__wrap_synth_operatorMultiple, channel, 0);
        expect_value(__wrap_synth_operatorMultiple, op, expectedOp);
        expect_value(__wrap_synth_operatorMultiple, multiple, expectedValue);

        __real_midi_cc(0, cc, 32);
    }
}

static void test_midi_sets_operator_detune(UNUSED void** state)
{
    const u8 expectedValue = 2;

    for (u8 cc = 24; cc <= 27; cc++) {
        u8 expectedOp = cc - 24;
        expect_value(__wrap_synth_operatorDetune, channel, 0);
        expect_value(__wrap_synth_operatorDetune, op, expectedOp);
        expect_value(__wrap_synth_operatorDetune, detune, expectedValue);

        __real_midi_cc(0, cc, 32);
    }
}

static void test_midi_sets_operator_rate_scaling(UNUSED void** state)
{
    const u8 expectedValue = 2;

    for (u8 cc = 39; cc <= 42; cc++) {
        u8 expectedOp = cc - 39;
        expect_value(__wrap_synth_operatorRateScaling, channel, 0);
        expect_value(__wrap_synth_operatorRateScaling, op, expectedOp);
        expect_value(
            __wrap_synth_operatorRateScaling, rateScaling, expectedValue);

        __real_midi_cc(0, cc, 64);
    }
}

static void test_midi_sets_operator_attack_rate(UNUSED void** state)
{
    const u8 expectedValue = 2;

    for (u8 cc = 43; cc <= 46; cc++) {
        u8 expectedOp = cc - 43;
        expect_value(__wrap_synth_operatorAttackRate, channel, 0);
        expect_value(__wrap_synth_operatorAttackRate, op, expectedOp);
        expect_value(
            __wrap_synth_operatorAttackRate, attackRate, expectedValue);

        __real_midi_cc(0, cc, 8);
    }
}

static void test_midi_sets_operator_first_decay_rate(UNUSED void** state)
{
    const u8 expectedValue = 2;

    for (u8 cc = 47; cc <= 50; cc++) {
        u8 expectedOp = cc - 47;
        expect_value(__wrap_synth_operatorFirstDecayRate, channel, 0);
        expect_value(__wrap_synth_operatorFirstDecayRate, op, expectedOp);
        expect_value(
            __wrap_synth_operatorFirstDecayRate, firstDecayRate, expectedValue);

        __real_midi_cc(0, cc, 8);
    }
}

static void test_midi_sets_operator_second_decay_rate(UNUSED void** state)
{
    const u8 expectedValue = 1;

    for (u8 cc = 51; cc <= 54; cc++) {
        u8 expectedOp = cc - 51;
        expect_value(__wrap_synth_operatorSecondDecayRate, channel, 0);
        expect_value(__wrap_synth_operatorSecondDecayRate, op, expectedOp);
        expect_value(__wrap_synth_operatorSecondDecayRate, secondDecayRate,
            expectedValue);

        __real_midi_cc(0, cc, 8);
    }
}

static void test_midi_sets_operator_secondary_amplitude(UNUSED void** state)
{
    const u8 expectedValue = 1;

    for (u8 cc = 55; cc <= 58; cc++) {
        u8 expectedOp = cc - 55;
        expect_value(__wrap_synth_operatorSecondaryAmplitude, channel, 0);
        expect_value(__wrap_synth_operatorSecondaryAmplitude, op, expectedOp);
        expect_value(__wrap_synth_operatorSecondaryAmplitude,
            secondaryAmplitude, expectedValue);

        __real_midi_cc(0, cc, 8);
    }
}

static void test_midi_sets_operator_amplitude_modulation(UNUSED void** state)
{
    const u8 expectedValue = 1;

    for (u8 cc = 70; cc <= 73; cc++) {
        u8 expectedOp = cc - 70;
        expect_value(__wrap_synth_operatorAmplitudeModulation, channel, 0);
        expect_value(__wrap_synth_operatorAmplitudeModulation, op, expectedOp);
        expect_value(__wrap_synth_operatorAmplitudeModulation,
            amplitudeModulation, expectedValue);

        __real_midi_cc(0, cc, 96);
    }
}

static void test_midi_sets_operator_release_rate(UNUSED void** state)
{
    const u8 expectedValue = 1;

    for (u8 cc = 59; cc <= 62; cc++) {
        u8 expectedOp = cc - 59;
        expect_value(__wrap_synth_operatorReleaseRate, channel, 0);
        expect_value(__wrap_synth_operatorReleaseRate, op, expectedOp);
        expect_value(
            __wrap_synth_operatorReleaseRate, releaseRate, expectedValue);

        __real_midi_cc(0, cc, 8);
    }
}

static void test_midi_sets_operator_ssg_eg(UNUSED void** state)
{
    const u8 expectedValue = 11;

    const u8 MIN_CC = 90;
    const u8 MAX_CC = 93;

    for (u8 cc = MIN_CC; cc <= MAX_CC; cc++) {
        u8 expectedOp = cc - MIN_CC;
        expect_value(__wrap_synth_operatorSsgEg, channel, 0);
        expect_value(__wrap_synth_operatorSsgEg, op, expectedOp);
        expect_value(__wrap_synth_operatorSsgEg, ssgEg, expectedValue);

        __real_midi_cc(0, cc, 88);
    }
}

static void test_midi_sets_global_LFO_enable(UNUSED void** state)
{
    expect_value(__wrap_synth_enableLfo, enable, 1);

    __real_midi_cc(0, 74, 64);
}

static void test_midi_sets_global_LFO_frequency(UNUSED void** state)
{
    expect_value(__wrap_synth_globalLfoFrequency, freq, 1);

    __real_midi_cc(0, 1, 16);
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
        expect_value(__wrap_synth_pitch, channel, chan);
        expect_value(__wrap_synth_pitch, octave, 6);
        expect_value(__wrap_synth_pitch, freqNumber, 1164);
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
    expect_value(__wrap_synth_noteOn, channel, 0);

    __real_midi_noteOn(0, A_SHARP, 127);

    assert_false(midi_overflow());
}

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

static void test_midi_sets_fm_preset(UNUSED void** state)
{
    const u8 program = 1;
    const u8 chan = 0;

    expect_value(__wrap_synth_preset, channel, chan);
    expect_value(__wrap_synth_preset, preset, program);

    __real_midi_program(chan, program);
}

static void test_midi_sysex_sends_all_notes_off(UNUSED void** state)
{
    u8 sysExGeneralMidiResetSequence[] = { 0x7E, 0x7F, 0x09, 0x01 };

    expect_value(__wrap_synth_noteOff, channel, 0);
    expect_value(__wrap_synth_noteOff, channel, 1);
    expect_value(__wrap_synth_noteOff, channel, 2);
    expect_value(__wrap_synth_noteOff, channel, 3);
    expect_value(__wrap_synth_noteOff, channel, 4);
    expect_value(__wrap_synth_noteOff, channel, 5);
    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_value(__wrap_psg_attenuation, attenuation, PSG_ATTENUATION_SILENCE);
    expect_value(__wrap_psg_attenuation, channel, 1);
    expect_value(__wrap_psg_attenuation, attenuation, PSG_ATTENUATION_SILENCE);
    expect_value(__wrap_psg_attenuation, channel, 2);
    expect_value(__wrap_psg_attenuation, attenuation, PSG_ATTENUATION_SILENCE);
    expect_value(__wrap_psg_attenuation, channel, 3);
    expect_value(__wrap_psg_attenuation, attenuation, PSG_ATTENUATION_SILENCE);

    __real_midi_sysex(
        sysExGeneralMidiResetSequence, sizeof(sysExGeneralMidiResetSequence));
}

static void test_midi_sysex_ignores_unknown_sysex(UNUSED void** state)
{
    u8 sysExGeneralMidiResetSequence[] = { 0x12 };

   __real_midi_sysex(
        sysExGeneralMidiResetSequence, sizeof(sysExGeneralMidiResetSequence));
}
