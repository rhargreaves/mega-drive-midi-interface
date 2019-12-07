#include "test_midi.h"

static void test_midi_triggers_synth_note_on(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        print_message("Chan %d\n", chan);
        expect_synth_pitch(chan, 4, 653);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_noteOn(chan, 60, 127);
    }
}

static void test_midi_triggers_synth_note_on_with_velocity(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, 653);
        expect_synth_volume(chan, 63);
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_noteOn(chan, 60, 63);
    }
}

static void test_midi_triggers_synth_note_on_with_velocity_and_channel_volume(
    UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_volume(chan, MAX_MIDI_VOLUME / 2);
        __real_midi_cc(chan, CC_VOLUME, MAX_MIDI_VOLUME / 2);

        expect_synth_pitch(chan, 4, 653);
        expect_synth_volume(chan, MAX_MIDI_VOLUME / 4);
        expect_value(__wrap_synth_noteOn, channel, chan);
        __real_midi_noteOn(chan, 60, MAX_MIDI_VOLUME / 2);
    }
}

static void test_midi_changing_volume_during_note_on_respects_velocity(
    UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, 653);
        expect_synth_volume(chan, MAX_MIDI_VOLUME / 2);
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_noteOn(chan, 60, MAX_MIDI_VOLUME / 2);

        expect_value(__wrap_synth_volume, channel, chan);
        expect_value(__wrap_synth_volume, volume, MAX_MIDI_VOLUME / 4);

        __real_midi_cc(chan, CC_VOLUME, MAX_MIDI_VOLUME / 2);
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
            expect_synth_pitch(
                chan, expectedOctaves[index], expectedFrequencies[index]);
            expect_synth_volume_any();
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
        print_message("Chan %d Note On\n", chan);
        expect_synth_pitch(chan, 4, 653);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_noteOn(chan, 60, 127);

        print_message("Chan %d Note Off\n", chan);
        expect_value(__wrap_synth_noteOff, channel, chan);

        __real_midi_noteOff(chan, 60);
    }
}

static void test_midi_triggers_synth_note_on_2(UNUSED void** state)
{
    expect_synth_pitch(0, 6, 1164);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);

    __real_midi_noteOn(0, A_SHARP, 127);
}

static void test_midi_channel_volume_sets_volume(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_synth_volume(chan, 60);
        __real_midi_cc(chan, CC_VOLUME, 60);
    }
}

static void test_midi_pan_sets_synth_stereo_mode_right(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_value(__wrap_synth_stereo, channel, chan);
        expect_value(__wrap_synth_stereo, mode, 1);

        __real_midi_cc(chan, CC_PAN, 127);
    }
}

static void test_midi_pan_sets_synth_stereo_mode_left(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_value(__wrap_synth_stereo, channel, chan);
        expect_value(__wrap_synth_stereo, mode, 2);

        __real_midi_cc(chan, CC_PAN, 0);
    }
}

static void test_midi_pan_sets_synth_stereo_mode_centre(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_value(__wrap_synth_stereo, channel, chan);
        expect_value(__wrap_synth_stereo, mode, 3);

        __real_midi_cc(chan, CC_PAN, 32);

        expect_value(__wrap_synth_stereo, channel, chan);
        expect_value(__wrap_synth_stereo, mode, 3);

        __real_midi_cc(chan, CC_PAN, 95);
    }
}

static void test_midi_sets_fm_algorithm(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_value(__wrap_synth_algorithm, channel, chan);
        expect_value(__wrap_synth_algorithm, algorithm, 1);

        __real_midi_cc(chan, CC_GENMDM_FM_ALGORITHM, 20);
    }
}

static void test_midi_sets_fm_feedback(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_value(__wrap_synth_feedback, channel, chan);
        expect_value(__wrap_synth_feedback, feedback, 2);

        __real_midi_cc(chan, CC_GENMDM_FM_FEEDBACK, 33);
    }
}

static void test_midi_sets_channel_AMS(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_value(__wrap_synth_ams, channel, chan);
        expect_value(__wrap_synth_ams, ams, 1);

        __real_midi_cc(chan, 76, 32);
    }
}

static void test_midi_sets_channel_FMS(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_value(__wrap_synth_fms, channel, chan);
        expect_value(__wrap_synth_fms, fms, 2);

        __real_midi_cc(chan, 75, 32);
    }
}

static void test_midi_sets_operator_total_level(UNUSED void** state)
{
    const u8 expectedValue = 50;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 16; cc <= 19; cc++) {
            u8 expectedOp = cc - 16;
            expect_value(__wrap_synth_operatorTotalLevel, channel, chan);
            expect_value(__wrap_synth_operatorTotalLevel, op, expectedOp);
            expect_value(
                __wrap_synth_operatorTotalLevel, totalLevel, expectedValue);

            __real_midi_cc(chan, cc, expectedValue);
        }
    }
}

static void test_midi_sets_operator_multiple(UNUSED void** state)
{
    const u8 expectedValue = 4;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 20; cc <= 23; cc++) {
            u8 expectedOp = cc - 20;
            expect_value(__wrap_synth_operatorMultiple, channel, chan);
            expect_value(__wrap_synth_operatorMultiple, op, expectedOp);
            expect_value(
                __wrap_synth_operatorMultiple, multiple, expectedValue);

            __real_midi_cc(chan, cc, 32);
        }
    }
}

static void test_midi_sets_operator_detune(UNUSED void** state)
{
    const u8 expectedValue = 2;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 24; cc <= 27; cc++) {
            u8 expectedOp = cc - 24;
            expect_value(__wrap_synth_operatorDetune, channel, chan);
            expect_value(__wrap_synth_operatorDetune, op, expectedOp);
            expect_value(__wrap_synth_operatorDetune, detune, expectedValue);

            __real_midi_cc(chan, cc, 32);
        }
    }
}

static void test_midi_sets_operator_rate_scaling(UNUSED void** state)
{
    const u8 expectedValue = 2;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 39; cc <= 42; cc++) {
            u8 expectedOp = cc - 39;
            expect_value(__wrap_synth_operatorRateScaling, channel, chan);
            expect_value(__wrap_synth_operatorRateScaling, op, expectedOp);
            expect_value(
                __wrap_synth_operatorRateScaling, rateScaling, expectedValue);

            __real_midi_cc(chan, cc, 64);
        }
    }
}

static void test_midi_sets_operator_attack_rate(UNUSED void** state)
{
    const u8 expectedValue = 2;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 43; cc <= 46; cc++) {
            u8 expectedOp = cc - 43;
            expect_value(__wrap_synth_operatorAttackRate, channel, chan);
            expect_value(__wrap_synth_operatorAttackRate, op, expectedOp);
            expect_value(
                __wrap_synth_operatorAttackRate, attackRate, expectedValue);

            __real_midi_cc(chan, cc, 8);
        }
    }
}

static void test_midi_sets_operator_first_decay_rate(UNUSED void** state)
{
    const u8 expectedValue = 2;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 47; cc <= 50; cc++) {
            u8 expectedOp = cc - 47;
            expect_value(__wrap_synth_operatorFirstDecayRate, channel, chan);
            expect_value(__wrap_synth_operatorFirstDecayRate, op, expectedOp);
            expect_value(__wrap_synth_operatorFirstDecayRate, firstDecayRate,
                expectedValue);

            __real_midi_cc(chan, cc, 8);
        }
    }
}

static void test_midi_sets_operator_second_decay_rate(UNUSED void** state)
{
    const u8 expectedValue = 1;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 51; cc <= 54; cc++) {
            u8 expectedOp = cc - 51;
            expect_value(__wrap_synth_operatorSecondDecayRate, channel, chan);
            expect_value(__wrap_synth_operatorSecondDecayRate, op, expectedOp);
            expect_value(__wrap_synth_operatorSecondDecayRate, secondDecayRate,
                expectedValue);

            __real_midi_cc(chan, cc, 8);
        }
    }
}

static void test_midi_sets_operator_secondary_amplitude(UNUSED void** state)
{
    const u8 expectedValue = 1;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 55; cc <= 58; cc++) {
            u8 expectedOp = cc - 55;
            expect_value(
                __wrap_synth_operatorSecondaryAmplitude, channel, chan);
            expect_value(
                __wrap_synth_operatorSecondaryAmplitude, op, expectedOp);
            expect_value(__wrap_synth_operatorSecondaryAmplitude,
                secondaryAmplitude, expectedValue);

            __real_midi_cc(chan, cc, 8);
        }
    }
}

static void test_midi_sets_operator_amplitude_modulation(UNUSED void** state)
{
    const u8 expectedValue = 1;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 70; cc <= 73; cc++) {
            u8 expectedOp = cc - 70;
            expect_value(
                __wrap_synth_operatorAmplitudeModulation, channel, chan);
            expect_value(
                __wrap_synth_operatorAmplitudeModulation, op, expectedOp);
            expect_value(__wrap_synth_operatorAmplitudeModulation,
                amplitudeModulation, expectedValue);

            __real_midi_cc(chan, cc, 96);
        }
    }
}

static void test_midi_sets_operator_release_rate(UNUSED void** state)
{
    const u8 expectedValue = 1;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 59; cc <= 62; cc++) {
            u8 expectedOp = cc - 59;
            expect_value(__wrap_synth_operatorReleaseRate, channel, chan);
            expect_value(__wrap_synth_operatorReleaseRate, op, expectedOp);
            expect_value(
                __wrap_synth_operatorReleaseRate, releaseRate, expectedValue);

            __real_midi_cc(chan, cc, 8);
        }
    }
}

static void test_midi_sets_operator_ssg_eg(UNUSED void** state)
{
    const u8 expectedValue = 11;

    const u8 MIN_CC = 90;
    const u8 MAX_CC = 93;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = MIN_CC; cc <= MAX_CC; cc++) {
            u8 expectedOp = cc - MIN_CC;
            expect_value(__wrap_synth_operatorSsgEg, channel, chan);
            expect_value(__wrap_synth_operatorSsgEg, op, expectedOp);
            expect_value(__wrap_synth_operatorSsgEg, ssgEg, expectedValue);

            __real_midi_cc(chan, cc, 88);
        }
    }
}

static void test_midi_sets_genmdm_stereo_mode(UNUSED void** state)
{
    const u8 expectedMode = 3;
    const u8 value = 127;
    const u8 cc = 77;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_value(__wrap_synth_stereo, channel, chan);
        expect_value(__wrap_synth_stereo, mode, expectedMode);

        __real_midi_cc(chan, cc, value);
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

static void test_midi_sets_fm_preset(UNUSED void** state)
{
    const u8 program = 1;
    const u8 chan = 0;

    const Channel M_BANK_0_INST_1_BRIGHTPIANO = { 5, 7, 3, 0, 0, 0, 0,
        { { 4, 2, 27, 1, 9, 0, 11, 5, 6, 33, 0 },
            { 4, 5, 27, 1, 9, 0, 7, 9, 7, 18, 0 },
            { 1, 2, 27, 1, 5, 1, 10, 5, 6, 8, 0 },
            { 6, 5, 27, 1, 9, 0, 3, 8, 7, 9, 0 } } };

    expect_value(__wrap_synth_preset, channel, chan);
    expect_memory(__wrap_synth_preset, preset, &M_BANK_0_INST_1_BRIGHTPIANO,
        sizeof(M_BANK_0_INST_1_BRIGHTPIANO));
    expect_value(__wrap_synth_stereo, channel, chan);
    expect_any(__wrap_synth_stereo, mode);

    __real_midi_program(chan, program);
}

static void test_midi_sets_synth_pitch_bend(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, 653);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);
        __real_midi_noteOn(chan, 60, 127);

        expect_synth_pitch(chan, 4, 0x22e);
        __real_midi_pitchBend(chan, 1000);
    }
}

static void remap_midi_channel(u8 midiChannel, u8 deviceChannel)
{
    u8 sequence[] = { SYSEX_EXTENDED_MANU_ID_SECTION,
        SYSEX_UNUSED_EUROPEAN_SECTION, SYSEX_UNUSED_MANU_ID,
        SYSEX_REMAP_COMMAND_ID, midiChannel, deviceChannel };

    __real_midi_sysex(sequence, sizeof(sequence) / sizeof(sequence[0]));
}

static void test_midi_fm_note_on_percussion_channel_sets_percussion_preset(
    UNUSED void** state)
{
    const u8 MIDI_PERCUSSION_CHANNEL = 9;
    const u8 FM_CHANNEL = 5;
    const u8 MIDI_KEY = 30;

    remap_midi_channel(MIDI_PERCUSSION_CHANNEL, FM_CHANNEL);

    const Channel P_BANK_0_INST_30_CASTANETS = { 4, 3, 3, 0, 0, 0, 0,
        { { 9, 0, 31, 0, 11, 0, 15, 0, 15, 23, 0 },
            { 1, 0, 31, 0, 19, 0, 15, 0, 15, 15, 0 },
            { 4, 0, 31, 2, 20, 0, 15, 0, 15, 13, 0 },
            { 2, 0, 31, 2, 20, 0, 15, 0, 15, 13, 0 } } };

    expect_value(__wrap_synth_preset, channel, FM_CHANNEL);
    expect_memory(__wrap_synth_preset, preset, &P_BANK_0_INST_30_CASTANETS,
        sizeof(P_BANK_0_INST_30_CASTANETS));

    expect_synth_volume_any();
    expect_synth_pitch(FM_CHANNEL, 0, 0x337);
    expect_value(__wrap_synth_noteOn, channel, FM_CHANNEL);

    __real_midi_noteOn(MIDI_PERCUSSION_CHANNEL, MIDI_KEY, MAX_MIDI_VOLUME);
}

static void test_midi_switching_program_retains_pan_setting(UNUSED void** state)
{
    const u8 program = 1;
    const u8 chan = 0;

    expect_value(__wrap_synth_stereo, channel, chan);
    expect_value(__wrap_synth_stereo, mode, 1);

    __real_midi_cc(0, CC_PAN, 127);

    const Channel M_BANK_0_INST_1_BRIGHTPIANO = { 5, 7, 3, 0, 0, 0, 0,
        { { 4, 2, 27, 1, 9, 0, 11, 5, 6, 33, 0 },
            { 4, 5, 27, 1, 9, 0, 7, 9, 7, 18, 0 },
            { 1, 2, 27, 1, 5, 1, 10, 5, 6, 8, 0 },
            { 6, 5, 27, 1, 9, 0, 3, 8, 7, 9, 0 } } };

    expect_value(__wrap_synth_preset, channel, chan);
    expect_memory(__wrap_synth_preset, preset, &M_BANK_0_INST_1_BRIGHTPIANO,
        sizeof(M_BANK_0_INST_1_BRIGHTPIANO));
    expect_value(__wrap_synth_stereo, channel, chan);
    expect_value(__wrap_synth_stereo, mode, 1);

    __real_midi_program(chan, program);
}
