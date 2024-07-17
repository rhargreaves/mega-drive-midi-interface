#include "test_midi.h"
#include "debug.h"

static void test_midi_triggers_synth_note_on(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);
    }
}

static void test_midi_triggers_synth_note_on_with_velocity(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume(chan, 63);
        expect_value(__wrap_synth_noteOn, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME / 2);
    }
}

static void test_midi_triggers_synth_note_on_with_velocity_and_channel_volume(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_volume(chan, MAX_MIDI_VOLUME / 2);
        __real_midi_cc(chan, CC_VOLUME, MAX_MIDI_VOLUME / 2);

        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume(chan, MAX_MIDI_VOLUME / 4);
        expect_value(__wrap_synth_noteOn, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME / 2);
    }
}

static void test_midi_changing_volume_during_note_on_respects_velocity(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume(chan, MAX_MIDI_VOLUME / 2);
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME / 2);

        expect_value(__wrap_synth_volume, channel, chan);
        expect_value(__wrap_synth_volume, volume, MAX_MIDI_VOLUME / 4);

        __real_midi_cc(chan, CC_VOLUME, MAX_MIDI_VOLUME / 2);
    }
}

static void test_midi_triggers_synth_note_on_boundary_values(UNUSED void** state)
{
    const u8 keys[] = { 11, 106 };
    const u16 expectedFrequencies[] = { 607, SYNTH_NTSC_AS };
    const u8 expectedOctaves[] = { 0, 7 };

    for (int index = 0; index < 2; index++) {
        for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
            expect_synth_pitch(chan, expectedOctaves[index], expectedFrequencies[index]);
            expect_synth_volume_any();
            expect_value(__wrap_synth_noteOn, channel, chan);

            __real_midi_note_on(chan, keys[index], MAX_MIDI_VOLUME);
        }
    }
}

static void test_midi_does_not_trigger_synth_note_on_out_of_bound_values(UNUSED void** state)
{
    const u8 keys[] = { 0, 10, 107, 127 };

    for (int index = 0; index < 2; index++) {
        for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
            __real_midi_note_on(chan, keys[index], MAX_MIDI_VOLUME);
        }
    }
}

static void test_midi_triggers_synth_note_off(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

        expect_value(__wrap_synth_noteOff, channel, chan);
        __real_midi_note_off(chan, MIDI_PITCH_C4);
    }
}

static void test_midi_triggers_synth_note_off_when_note_on_has_zero_velocity(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

        expect_value(__wrap_synth_noteOff, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_C4, 0);
    }
}

static void test_midi_triggers_synth_note_on_2(UNUSED void** state)
{
    expect_synth_pitch(0, 6, SYNTH_NTSC_AS);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);

    __real_midi_note_on(0, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);
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

        __real_midi_cc(chan, CC_PAN, 96);
    }
}

static void test_midi_pan_sets_synth_stereo_mode_left(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_value(__wrap_synth_stereo, channel, chan);
        expect_value(__wrap_synth_stereo, mode, 2);

        __real_midi_cc(chan, CC_PAN, 31);
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
            expect_value(__wrap_synth_operatorTotalLevel, totalLevel, expectedValue);

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
            expect_value(__wrap_synth_operatorMultiple, multiple, expectedValue);

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
            expect_value(__wrap_synth_operatorRateScaling, rateScaling, expectedValue);

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
            expect_value(__wrap_synth_operatorAttackRate, attackRate, expectedValue);

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
            expect_value(__wrap_synth_operatorFirstDecayRate, firstDecayRate, expectedValue);

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
            expect_value(__wrap_synth_operatorSecondDecayRate, secondDecayRate, expectedValue);

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
            expect_value(__wrap_synth_operatorSecondaryAmplitude, channel, chan);
            expect_value(__wrap_synth_operatorSecondaryAmplitude, op, expectedOp);
            expect_value(
                __wrap_synth_operatorSecondaryAmplitude, secondaryAmplitude, expectedValue);

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
            expect_value(__wrap_synth_operatorAmplitudeModulation, channel, chan);
            expect_value(__wrap_synth_operatorAmplitudeModulation, op, expectedOp);
            expect_value(
                __wrap_synth_operatorAmplitudeModulation, amplitudeModulation, expectedValue);

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
            expect_value(__wrap_synth_operatorReleaseRate, releaseRate, expectedValue);

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

    const FmChannel M_BANK_0_INST_1_BRIGHTPIANO = { 5, 7, 3, 0, 0, 0, 0,
        { { 4, 2, 27, 1, 9, 0, 11, 5, 6, 33, 0 }, { 4, 5, 27, 1, 9, 0, 7, 9, 7, 18, 0 },
            { 1, 2, 27, 1, 5, 1, 10, 5, 6, 8, 0 }, { 6, 5, 27, 1, 9, 0, 3, 8, 7, 9, 0 } } };

    expect_value(__wrap_synth_preset, channel, chan);
    expect_memory(__wrap_synth_preset, preset, &M_BANK_0_INST_1_BRIGHTPIANO,
        sizeof(M_BANK_0_INST_1_BRIGHTPIANO));

    __real_midi_program(chan, program);
}

static void test_midi_sets_synth_pitch_bend(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);
        __real_midi_note_on(chan, 60, MAX_MIDI_VOLUME);

        expect_synth_pitch(chan, 4, 0x246);
        __real_midi_pitch_bend(chan, 1000);
    }
}

static void test_midi_pitch_bends_down_an_octave(UNUSED void** state)
{
    const u8 MIDI_PITCH_B3 = 59;
    const u16 SYNTH_FREQ_B3 = 0x25f;
    const u16 SYNTH_FREQ_A3 = 541;

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_FREQ_B3);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_B3, MAX_MIDI_VOLUME);

        expect_synth_pitch(chan, 4, SYNTH_FREQ_A3);
        __real_midi_pitch_bend(chan, MIDI_PITCH_BEND_MIN); // down 2 st
    }
}

static void test_midi_pitch_bends_up_an_octave(UNUSED void** state)
{
    const u8 MIDI_PITCH_B3 = 59;
    const u16 SYNTH_FREQ_B3 = 0x25f;
    const u16 SYNTH_FREQ_C3 = 681;

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_FREQ_B3);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_B3, MAX_MIDI_VOLUME);

        expect_synth_pitch(chan, 4, SYNTH_FREQ_C3);
        __real_midi_pitch_bend(chan, MIDI_PITCH_BEND_MAX); // up 2 st
    }
}

static void test_midi_pitch_bends_up_an_octave_upper_freq_limit(UNUSED void** state)
{
    const u8 MIDI_PITCH_AS4 = 70;
    const u16 SYNTH_FREQ_AS4 = 1146;
    const u16 SYNTH_FREQ_C3 = 1286;

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_FREQ_AS4);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_AS4, MAX_MIDI_VOLUME);

        expect_synth_pitch(chan, 4, SYNTH_FREQ_C3);
        __real_midi_pitch_bend(chan, MIDI_PITCH_BEND_MAX); // up 2 st
    }
}

static void test_midi_persists_pitch_bend_between_notes(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);
        __real_midi_note_on(chan, 60, MAX_MIDI_VOLUME);

        expect_synth_pitch(chan, 4, 0x246);
        __real_midi_pitch_bend(chan, 1000);

        expect_synth_pitch(chan, 4, 0x246);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);
        __real_midi_note_on(chan, 60, MAX_MIDI_VOLUME);
    }
}

static void remap_midi_channel(u8 midiChannel, u8 deviceChannel)
{
    u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID, SYSEX_COMMAND_REMAP,
        midiChannel, deviceChannel };

    __real_midi_sysex(sequence, sizeof(sequence) / sizeof(sequence[0]));
}

static void test_midi_fm_note_on_percussion_channel_sets_percussion_preset(UNUSED void** state)
{
    const u8 MIDI_PERCUSSION_CHANNEL = 9;
    const u8 FM_CHANNEL = 5;
    const u8 MIDI_KEY = 30;

    remap_midi_channel(MIDI_PERCUSSION_CHANNEL, FM_CHANNEL);

    const FmChannel P_BANK_0_INST_30_CASTANETS = { 4, 3, 3, 0, 0, 0, 0,
        { { 9, 0, 31, 0, 11, 0, 15, 0, 15, 23, 0 }, { 1, 0, 31, 0, 19, 0, 15, 0, 15, 15, 0 },
            { 4, 0, 31, 2, 20, 0, 15, 0, 15, 13, 0 }, { 2, 0, 31, 2, 20, 0, 15, 0, 15, 13, 0 } } };

    expect_value(__wrap_synth_preset, channel, FM_CHANNEL);
    expect_memory(__wrap_synth_preset, preset, &P_BANK_0_INST_30_CASTANETS,
        sizeof(P_BANK_0_INST_30_CASTANETS));

    expect_synth_volume_any();
    expect_synth_pitch(FM_CHANNEL, 0, 0x32a);
    expect_value(__wrap_synth_noteOn, channel, FM_CHANNEL);

    __real_midi_note_on(MIDI_PERCUSSION_CHANNEL, MIDI_KEY, MAX_MIDI_VOLUME);
}

static void test_midi_switching_program_retains_pan_setting(UNUSED void** state)
{
    const u8 program = 1;
    const u8 chan = 0;

    expect_value(__wrap_synth_stereo, channel, chan);
    expect_value(__wrap_synth_stereo, mode, 1);
    __real_midi_cc(0, CC_PAN, 127);

    const FmChannel M_BANK_0_INST_1_BRIGHTPIANO = { 5, 7, 3, 0, 0, 0, 0,
        { { 4, 2, 27, 1, 9, 0, 11, 5, 6, 33, 0 }, { 4, 5, 27, 1, 9, 0, 7, 9, 7, 18, 0 },
            { 1, 2, 27, 1, 5, 1, 10, 5, 6, 8, 0 }, { 6, 5, 27, 1, 9, 0, 3, 8, 7, 9, 0 } } };

    expect_value(__wrap_synth_preset, channel, chan);
    expect_memory(__wrap_synth_preset, preset, &M_BANK_0_INST_1_BRIGHTPIANO,
        sizeof(M_BANK_0_INST_1_BRIGHTPIANO));
    __real_midi_program(chan, program);
}

static void test_midi_enables_fm_special_mode(UNUSED void** state)
{
    u8 expectedController = 80;
    u8 expectedValue = 64;

    expect_value(__wrap_synth_setSpecialMode, enable, true);

    __real_midi_cc(0, expectedController, expectedValue);
}

static void test_midi_disables_fm_special_mode(UNUSED void** state)
{
    u8 expectedController = 80;
    u8 expectedValue = 0;

    expect_value(__wrap_synth_setSpecialMode, enable, false);

    __real_midi_cc(0, expectedController, expectedValue);
}

static void test_midi_sets_pitch_of_special_mode_ch3_operator(UNUSED void** state)
{
    for (u8 op = 0; op < 3; op++) {
        int midiChannel = 10 + op;

        expect_value(__wrap_synth_specialModePitch, op, op);
        expect_value(__wrap_synth_specialModePitch, octave, 4);
        expect_value(__wrap_synth_specialModePitch, freqNumber, SYNTH_NTSC_C);

        expect_value(__wrap_synth_specialModeVolume, op, op);
        expect_value(__wrap_synth_specialModeVolume, volume, MAX_MIDI_VOLUME);

        __real_midi_note_on(midiChannel, MIDI_PITCH_C4, MAX_MIDI_VOLUME);
    }
}

static void test_midi_sets_volume_of_special_mode_ch3_operator(UNUSED void** state)
{
    for (u8 op = 0; op < 3; op++) {
        int midiChannel = 10 + op;

        expect_value(__wrap_synth_specialModePitch, op, op);
        expect_value(__wrap_synth_specialModePitch, octave, 4);
        expect_value(__wrap_synth_specialModePitch, freqNumber, SYNTH_NTSC_C);

        expect_value(__wrap_synth_specialModeVolume, op, op);
        expect_value(__wrap_synth_specialModeVolume, volume, 64);

        __real_midi_note_on(midiChannel, MIDI_PITCH_C4, 64);
    }
}

static void test_midi_pitch_bends_special_mode_operator(UNUSED void** state)
{
    int chan = 10;

    expect_value(__wrap_synth_specialModePitch, op, 0);
    expect_value(__wrap_synth_specialModePitch, octave, 4);
    expect_value(__wrap_synth_specialModePitch, freqNumber, SYNTH_NTSC_C);
    expect_value(__wrap_synth_specialModeVolume, op, 0);
    expect_value(__wrap_synth_specialModeVolume, volume, MAX_MIDI_VOLUME);
    __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_value(__wrap_synth_specialModePitch, op, 0);
    expect_value(__wrap_synth_specialModePitch, octave, 4);
    expect_value(__wrap_synth_specialModePitch, freqNumber, 0x320);
    __real_midi_pitch_bend(chan, 0x6000);
}

static void test_midi_pitch_bends_special_mode_op_independent_of_other_ops(UNUSED void** state)
{
    expect_value(__wrap_synth_specialModePitch, op, 0);
    expect_value(__wrap_synth_specialModePitch, octave, 4);
    expect_value(__wrap_synth_specialModePitch, freqNumber, SYNTH_NTSC_C);
    expect_value(__wrap_synth_specialModeVolume, op, 0);
    expect_value(__wrap_synth_specialModeVolume, volume, MAX_MIDI_VOLUME);
    __real_midi_note_on(10, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_value(__wrap_synth_specialModePitch, op, 0);
    expect_value(__wrap_synth_specialModePitch, octave, 4);
    expect_value(__wrap_synth_specialModePitch, freqNumber, 0x320);
    __real_midi_pitch_bend(10, 0x6000);

    expect_value(__wrap_synth_specialModePitch, op, 1);
    expect_value(__wrap_synth_specialModePitch, octave, 4);
    expect_value(__wrap_synth_specialModePitch, freqNumber, SYNTH_NTSC_C);
    expect_value(__wrap_synth_specialModeVolume, op, 1);
    expect_value(__wrap_synth_specialModeVolume, volume, MAX_MIDI_VOLUME);
    __real_midi_note_on(11, MIDI_PITCH_C4, MAX_MIDI_VOLUME);
}

static void test_midi_persists_pitch_bends_for_special_mode_op_between_notes(UNUSED void** state)
{
    int chan = 10;

    expect_value(__wrap_synth_specialModePitch, op, 0);
    expect_value(__wrap_synth_specialModePitch, octave, 4);
    expect_value(__wrap_synth_specialModePitch, freqNumber, SYNTH_NTSC_C);
    expect_value(__wrap_synth_specialModeVolume, op, 0);
    expect_value(__wrap_synth_specialModeVolume, volume, MAX_MIDI_VOLUME);
    __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_value(__wrap_synth_specialModePitch, op, 0);
    expect_value(__wrap_synth_specialModePitch, octave, 4);
    expect_value(__wrap_synth_specialModePitch, freqNumber, 0x320);
    __real_midi_pitch_bend(chan, 0x6000);

    expect_value(__wrap_synth_specialModePitch, op, 0);
    expect_value(__wrap_synth_specialModePitch, octave, 4);
    expect_value(__wrap_synth_specialModePitch, freqNumber, 0x320);
    expect_value(__wrap_synth_specialModeVolume, op, 0);
    expect_value(__wrap_synth_specialModeVolume, volume, MAX_MIDI_VOLUME);
    __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);
}

static void test_midi_note_priority_respected_for_multiple_notes(UNUSED void** state)
{
    expect_synth_pitch(0, 4, 0x284);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);
    __real_midi_note_on(0, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_pitch(0, 4, 0x2a9);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);
    __real_midi_note_on(0, MIDI_PITCH_CS4, MAX_MIDI_VOLUME);

    expect_synth_pitch(0, 6, 0x47a);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);
    __real_midi_note_on(0, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);

    expect_synth_pitch(0, 4, 0x2a9);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);
    __real_midi_note_off(0, MIDI_PITCH_AS6);

    expect_synth_pitch(0, 4, 0x284);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);
    __real_midi_note_off(0, MIDI_PITCH_CS4);
}

#define NOTE_PRIORITY_LENGTH 10

static void test_midi_drops_note_when_note_priority_stack_full(UNUSED void** state)
{
    const u16 expectedFreqNum[NOTE_PRIORITY_LENGTH]
        = { 0x25f, 0x284, 0x2a9, 0x2d2, 0x2fd, 0x32a, 0x35a, 0x38e, 0x3c4, 0x3fd };
    const u8 expectedOctave = 4;

    for (u16 i = 0; i < NOTE_PRIORITY_LENGTH; i++) {
        u8 pitch = 59 + i;
        u16 freqNum = expectedFreqNum[i];

        print_message(
            "noteOn: pitch = %d, freqNum = 0x%x, octave = %d\n", pitch, freqNum, expectedOctave);

        expect_synth_pitch(0, expectedOctave, freqNum);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, 0);
        __real_midi_note_on(0, pitch, MAX_MIDI_VOLUME);
    }

    __real_midi_note_on(0, 100, MAX_MIDI_VOLUME);
}
