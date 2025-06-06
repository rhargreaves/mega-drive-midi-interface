#include "test_midi_fm.h"
#include "test_midi.h"
#include "mocks/mock_midi.h"
#include "utils.h"

void test_midi_triggers_synth_note_on(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume_any();
        expect_value(__wrap_synth_note_on, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);
    }
}

void test_midi_triggers_synth_note_on_with_velocity(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume(chan, 63);
        expect_value(__wrap_synth_note_on, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME / 2);
    }
}

void test_midi_triggers_synth_note_on_with_velocity_and_channel_volume(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_volume(chan, MAX_MIDI_VOLUME / 2);
        __real_midi_cc(chan, CC_VOLUME, MAX_MIDI_VOLUME / 2);

        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume(chan, MAX_MIDI_VOLUME / 4);
        expect_value(__wrap_synth_note_on, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME / 2);
    }
}

void test_midi_changing_volume_during_note_on_respects_velocity(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume(chan, MAX_MIDI_VOLUME / 2);
        expect_value(__wrap_synth_note_on, channel, chan);

        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME / 2);

        expect_value(__wrap_synth_volume, channel, chan);
        expect_value(__wrap_synth_volume, volume, MAX_MIDI_VOLUME / 4);

        __real_midi_cc(chan, CC_VOLUME, MAX_MIDI_VOLUME / 2);
    }
}

void test_midi_triggers_synth_note_on_boundary_values(UNUSED void** state)
{
    const u8 keys[] = { 11, 106 };
    const u16 expectedFrequencies[] = { 607, SYNTH_NTSC_AS };
    const u8 expectedOctaves[] = { 0, 7 };

    for (int index = 0; index < 2; index++) {
        for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
            expect_synth_pitch(chan, expectedOctaves[index], expectedFrequencies[index]);
            expect_synth_volume_any();
            expect_value(__wrap_synth_note_on, channel, chan);

            __real_midi_note_on(chan, keys[index], MAX_MIDI_VOLUME);
        }
    }
}

void test_midi_does_not_trigger_synth_note_on_out_of_bound_values(UNUSED void** state)
{
    const u8 keys[] = { 0, 10, 107, 127 };

    for (int index = 0; index < 2; index++) {
        for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
            __real_midi_note_on(chan, keys[index], MAX_MIDI_VOLUME);
        }
    }
}

void test_midi_triggers_synth_note_off(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume_any();
        expect_value(__wrap_synth_note_on, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

        expect_value(__wrap_synth_note_off, channel, chan);
        __real_midi_note_off(chan, MIDI_PITCH_C4);
    }
}

void test_midi_triggers_synth_note_off_when_note_on_has_zero_velocity(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume_any();
        expect_value(__wrap_synth_note_on, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

        expect_value(__wrap_synth_note_off, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_C4, 0);
    }
}

void test_midi_triggers_synth_note_on_2(UNUSED void** state)
{
    expect_synth_pitch(0, 6, SYNTH_NTSC_AS);
    expect_synth_volume_any();
    expect_value(__wrap_synth_note_on, channel, 0);

    __real_midi_note_on(0, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);
}

void test_midi_channel_volume_sets_volume(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_synth_volume(chan, 60);
        __real_midi_cc(chan, CC_VOLUME, 60);
    }
}

void test_midi_pan_sets_synth_stereo_mode_right(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_value(__wrap_synth_stereo, channel, chan);
        expect_value(__wrap_synth_stereo, mode, 1);

        __real_midi_cc(chan, CC_PAN, 96);
    }
}

void test_midi_pan_sets_synth_stereo_mode_left(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_value(__wrap_synth_stereo, channel, chan);
        expect_value(__wrap_synth_stereo, mode, 2);

        __real_midi_cc(chan, CC_PAN, 31);
    }
}

void test_midi_pan_sets_synth_stereo_mode_centre(UNUSED void** state)
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

void test_midi_sets_fm_algorithm(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_value(__wrap_synth_algorithm, channel, chan);
        expect_value(__wrap_synth_algorithm, algorithm, 1);

        __real_midi_cc(chan, CC_GENMDM_FM_ALGORITHM, 20);
    }
}

void test_midi_sets_fm_feedback(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_value(__wrap_synth_feedback, channel, chan);
        expect_value(__wrap_synth_feedback, feedback, 2);

        __real_midi_cc(chan, CC_GENMDM_FM_FEEDBACK, 33);
    }
}

void test_midi_sets_channel_AMS(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_synth_ams(chan, 1);
        __real_midi_cc(chan, 76, 32);
    }
}

void test_midi_sets_channel_FMS(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_synth_fms(chan, 2);
        __real_midi_cc(chan, 75, 32);
    }
}

void test_midi_sets_operator_total_level(UNUSED void** state)
{
    const u8 expectedValue = 50;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 16; cc <= 19; cc++) {
            u8 expectedOp = cc - 16;
            expect_synth_operator_total_level(chan, expectedOp, expectedValue);
            __real_midi_cc(chan, cc, expectedValue);
        }
    }
}

void test_midi_sets_operator_multiple(UNUSED void** state)
{
    const u8 expectedValue = 4;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 20; cc <= 23; cc++) {
            u8 expectedOp = cc - 20;
            expect_synth_operator_multiple(chan, expectedOp, expectedValue);
            __real_midi_cc(chan, cc, 32);
        }
    }
}

void test_midi_sets_operator_detune(UNUSED void** state)
{
    const u8 expectedValue = 2;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 24; cc <= 27; cc++) {
            u8 expectedOp = cc - 24;
            expect_synth_operator_detune(chan, expectedOp, expectedValue);
            __real_midi_cc(chan, cc, 32);
        }
    }
}

void test_midi_sets_operator_rate_scaling(UNUSED void** state)
{
    const u8 expectedValue = 2;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 39; cc <= 42; cc++) {
            u8 expectedOp = cc - 39;
            expect_synth_operator_rate_scaling(chan, expectedOp, expectedValue);
            __real_midi_cc(chan, cc, 64);
        }
    }
}

void test_midi_sets_operator_attack_rate(UNUSED void** state)
{
    const u8 expectedValue = 2;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 43; cc <= 46; cc++) {
            u8 expectedOp = cc - 43;
            expect_synth_operator_attack_rate(chan, expectedOp, expectedValue);
            __real_midi_cc(chan, cc, 8);
        }
    }
}

void test_midi_sets_operator_decay_rate(UNUSED void** state)
{
    const u8 expectedValue = 2;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 47; cc <= 50; cc++) {
            u8 expectedOp = cc - 47;
            expect_synth_operator_decay_rate(chan, expectedOp, expectedValue);
            __real_midi_cc(chan, cc, 8);
        }
    }
}

void test_midi_sets_operator_sustain_rate(UNUSED void** state)
{
    const u8 expectedValue = 1;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 51; cc <= 54; cc++) {
            u8 expectedOp = cc - 51;
            expect_synth_operator_sustain_rate(chan, expectedOp, expectedValue);
            __real_midi_cc(chan, cc, 8);
        }
    }
}

void test_midi_sets_operator_sustain_level(UNUSED void** state)
{
    const u8 expectedValue = 1;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 55; cc <= 58; cc++) {
            u8 expectedOp = cc - 55;
            expect_synth_operator_sustain_level(chan, expectedOp, expectedValue);
            __real_midi_cc(chan, cc, 8);
        }
    }
}

void test_midi_sets_operator_amplitude_modulation(UNUSED void** state)
{
    const u8 expectedValue = 1;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 70; cc <= 73; cc++) {
            u8 expectedOp = cc - 70;
            expect_synth_operator_amplitude_modulation(chan, expectedOp, expectedValue);

            __real_midi_cc(chan, cc, 96);
        }
    }
}

void test_midi_sets_operator_release_rate(UNUSED void** state)
{
    const u8 expectedValue = 1;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = 59; cc <= 62; cc++) {
            u8 expectedOp = cc - 59;
            expect_synth_operator_release_rate(chan, expectedOp, expectedValue);
            __real_midi_cc(chan, cc, 8);
        }
    }
}

void test_midi_sets_operator_ssg_eg(UNUSED void** state)
{
    const u8 expectedValue = 11;

    const u8 MIN_CC = 90;
    const u8 MAX_CC = 93;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 cc = MIN_CC; cc <= MAX_CC; cc++) {
            u8 expectedOp = cc - MIN_CC;
            expect_synth_operator_ssg_eg(chan, expectedOp, expectedValue);
            __real_midi_cc(chan, cc, 88);
        }
    }
}

void test_midi_sets_genmdm_stereo_mode(UNUSED void** state)
{
    const u8 expectedMode = 3;
    const u8 value = 127;
    const u8 cc = 77;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_synth_stereo(chan, expectedMode);
        __real_midi_cc(chan, cc, value);
    }
}

void test_midi_sets_global_LFO_enable(UNUSED void** state)
{
    expect_synth_enable_lfo(1);
    __real_midi_cc(0, 74, 64);
}

void test_midi_sets_global_LFO_frequency(UNUSED void** state)
{
    expect_synth_global_lfo_frequency(1);
    __real_midi_cc(0, 1, 16);
}

void test_midi_sets_fm_preset(UNUSED void** state)
{
    const u8 program = 1;
    const u8 chan = 0;

    const FmPreset M_BANK_0_INST_1_BRIGHTPIANO = { 5, 7, 0, 0,
        { { 4, 2, 27, 1, 9, 0, 11, 5, 6, 33, 0 }, { 4, 5, 27, 1, 9, 0, 7, 9, 7, 18, 0 },
            { 1, 2, 27, 1, 5, 1, 10, 5, 6, 8, 0 }, { 6, 5, 27, 1, 9, 0, 3, 8, 7, 9, 0 } } };
    expect_synth_preset(chan, &M_BANK_0_INST_1_BRIGHTPIANO);

    __real_midi_program(chan, program);
}

void test_midi_sets_synth_pitch_bend(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume_any();
        expect_synth_note_on(chan);
        __real_midi_note_on(chan, 60, MAX_MIDI_VOLUME);

        expect_synth_pitch(chan, 3, 0x48b);
        __real_midi_pitch_bend(chan, 1000);
    }
}

void test_midi_sets_synth_pitch_bend_before_note_on(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        print_message("channel %d\n", chan);
        expect_synth_pitch(chan, 3, 0x48b);
        __real_midi_pitch_bend(chan, 1000);

        print_message("note on %d\n", chan);
        expect_synth_pitch(chan, 3, 0x48b);
        expect_synth_volume_any();
        expect_synth_note_on(chan);
        __real_midi_note_on(chan, 60, MAX_MIDI_VOLUME);
    }
}

void test_midi_pitch_bends_down_an_octave(UNUSED void** state)
{
    const u16 SYNTH_FREQ_B3 = 0x25f;
    const u16 SYNTH_FREQ_A3 = 0x439;

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_FREQ_B3);
        expect_synth_volume_any();
        expect_synth_note_on(chan);
        __real_midi_note_on(chan, MIDI_PITCH_B3, MAX_MIDI_VOLUME);

        expect_synth_pitch(chan, 3, SYNTH_FREQ_A3);
        __real_midi_pitch_bend(chan, MIDI_PITCH_BEND_MIN); // down 2 st
    }
}

void test_midi_pitch_bends_up_an_octave(UNUSED void** state)
{
    const u16 SYNTH_FREQ_B3 = 0x25f;
    const u16 SYNTH_FREQ_C3 = 681;

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_FREQ_B3);
        expect_synth_volume_any();
        expect_synth_note_on(chan);
        __real_midi_note_on(chan, MIDI_PITCH_B3, MAX_MIDI_VOLUME);

        expect_synth_pitch(chan, 4, SYNTH_FREQ_C3);
        __real_midi_pitch_bend(chan, MIDI_PITCH_BEND_MAX); // up 2 st
    }
}

void test_midi_pitch_bends_up_an_octave_upper_freq_limit(UNUSED void** state)
{
    const u16 SYNTH_FREQ_AS4 = 1146;
    const u16 SYNTH_FREQ_C3 = 644;

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_FREQ_AS4);
        expect_synth_volume_any();
        expect_synth_note_on(chan);
        __real_midi_note_on(chan, MIDI_PITCH_AS4, MAX_MIDI_VOLUME);

        expect_synth_pitch(chan, 5, SYNTH_FREQ_C3);
        __real_midi_pitch_bend(chan, MIDI_PITCH_BEND_MAX); // up 2 st
    }
}

void test_midi_persists_pitch_bend_between_notes(UNUSED void** state)
{
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 4, SYNTH_NTSC_C);
        expect_synth_volume_any();
        expect_synth_note_on(chan);
        __real_midi_note_on(chan, 60, MAX_MIDI_VOLUME);

        expect_synth_pitch(chan, 3, 1163);
        __real_midi_pitch_bend(chan, 1000);

        expect_synth_note_off(chan);
        __real_midi_note_off(chan, 60);

        expect_synth_pitch(chan, 3, 1163);
        expect_synth_volume_any();
        expect_synth_note_on(chan);
        __real_midi_note_on(chan, 60, MAX_MIDI_VOLUME);
    }
}

static void remap_midi_channel(u8 midiChannel, u8 deviceChannel)
{
    u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID, SYSEX_COMMAND_REMAP,
        midiChannel, deviceChannel };

    __real_midi_sysex(sequence, sizeof(sequence) / sizeof(sequence[0]));
}

void test_midi_fm_note_on_percussion_channel_sets_percussion_preset(UNUSED void** state)
{
    const u8 MIDI_PERCUSSION_CHANNEL = 9;
    const u8 FM_CHANNEL = 5;
    const u8 MIDI_KEY = 30;

    remap_midi_channel(MIDI_PERCUSSION_CHANNEL, FM_CHANNEL);

    const FmPreset P_BANK_0_INST_30_CASTANETS = { 4, 3, 0, 0,
        { { 9, 0, 31, 0, 11, 0, 15, 0, 15, 23, 0 }, { 1, 0, 31, 0, 19, 0, 15, 0, 15, 15, 0 },
            { 4, 0, 31, 2, 20, 0, 15, 0, 15, 13, 0 }, { 2, 0, 31, 2, 20, 0, 15, 0, 15, 13, 0 } } };

    expect_synth_preset(FM_CHANNEL, &P_BANK_0_INST_30_CASTANETS);
    expect_synth_volume_any();
    expect_synth_pitch(FM_CHANNEL, 0, 0x32a);
    expect_synth_note_on(FM_CHANNEL);

    __real_midi_note_on(MIDI_PERCUSSION_CHANNEL, MIDI_KEY, MAX_MIDI_VOLUME);
}

void test_midi_switching_program_retains_pan_setting(UNUSED void** state)
{
    const u8 program = 1;
    const u8 chan = 0;

    expect_synth_stereo(chan, 1);
    __real_midi_cc(0, CC_PAN, 127);

    const FmPreset M_BANK_0_INST_1_BRIGHTPIANO = { 5, 7, 0, 0,
        { { 4, 2, 27, 1, 9, 0, 11, 5, 6, 33, 0 }, { 4, 5, 27, 1, 9, 0, 7, 9, 7, 18, 0 },
            { 1, 2, 27, 1, 5, 1, 10, 5, 6, 8, 0 }, { 6, 5, 27, 1, 9, 0, 3, 8, 7, 9, 0 } } };
    expect_synth_preset(chan, &M_BANK_0_INST_1_BRIGHTPIANO);

    __real_midi_program(chan, program);
}

void test_midi_enables_fm_special_mode(UNUSED void** state)
{
    u8 expectedController = 80;
    u8 expectedValue = 64;

    expect_synth_set_special_mode(true);

    __real_midi_cc(0, expectedController, expectedValue);
}

void test_midi_disables_fm_special_mode(UNUSED void** state)
{
    u8 expectedController = 80;
    u8 expectedValue = 0;

    expect_synth_set_special_mode(false);

    __real_midi_cc(0, expectedController, expectedValue);
}

void test_midi_sets_pitch_of_special_mode_ch3_operator(UNUSED void** state)
{
    for (u8 op = 0; op < 3; op++) {
        int midiChannel = 10 + op;

        expect_synth_special_mode_pitch(op, 4, SYNTH_NTSC_C);
        expect_synth_special_mode_volume(op, MAX_MIDI_VOLUME);
        __real_midi_note_on(midiChannel, MIDI_PITCH_C4, MAX_MIDI_VOLUME);
    }
}

void test_midi_sets_volume_of_special_mode_ch3_operator(UNUSED void** state)
{
    for (u8 op = 0; op < 3; op++) {
        int midiChannel = 10 + op;

        expect_synth_special_mode_pitch(op, 4, SYNTH_NTSC_C);
        expect_synth_special_mode_volume(op, 64);

        __real_midi_note_on(midiChannel, MIDI_PITCH_C4, 64);
    }
}

void test_midi_pitch_bends_special_mode_operator(UNUSED void** state)
{
    int chan = 10;

    expect_synth_special_mode_pitch(0, 4, SYNTH_NTSC_C);
    expect_synth_special_mode_volume(0, MAX_MIDI_VOLUME);
    __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_special_mode_pitch(0, 4, 0x32a);
    __real_midi_pitch_bend(chan, 0x6000);
}

void test_midi_pitch_bends_special_mode_op_independent_of_other_ops(UNUSED void** state)
{
    expect_synth_special_mode_pitch(0, 4, SYNTH_NTSC_C);
    expect_synth_special_mode_volume(0, MAX_MIDI_VOLUME);
    __real_midi_note_on(10, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_special_mode_pitch(0, 4, 0x32a);
    __real_midi_pitch_bend(10, 0x6000);

    expect_synth_special_mode_pitch(1, 4, SYNTH_NTSC_C);
    expect_synth_special_mode_volume(1, MAX_MIDI_VOLUME);
    __real_midi_note_on(11, MIDI_PITCH_C4, MAX_MIDI_VOLUME);
}

void test_midi_persists_pitch_bends_for_special_mode_op_between_notes(UNUSED void** state)
{
    int chan = 10;

    expect_synth_special_mode_pitch(0, 4, SYNTH_NTSC_C);
    expect_synth_special_mode_volume(0, MAX_MIDI_VOLUME);
    __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_special_mode_pitch(0, 4, 0x32a);
    __real_midi_pitch_bend(chan, 0x6000);

    expect_synth_special_mode_pitch(0, 4, 0x32a);
    expect_synth_special_mode_volume(0, MAX_MIDI_VOLUME);
    __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);
}

void test_midi_note_priority_respected_for_multiple_notes(UNUSED void** state)
{
    expect_synth_pitch(0, 4, 0x284);
    expect_synth_volume_any();
    expect_synth_note_on(0);
    __real_midi_note_on(0, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_pitch(0, 4, 0x2a9);
    expect_synth_volume_any();
    expect_synth_note_on(0);
    __real_midi_note_on(0, MIDI_PITCH_CS4, MAX_MIDI_VOLUME);

    expect_synth_pitch(0, 6, 0x47a);
    expect_synth_volume_any();
    expect_synth_note_on(0);
    __real_midi_note_on(0, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);

    expect_synth_pitch(0, 4, 0x2a9);
    expect_synth_volume_any();
    expect_synth_note_on(0);
    __real_midi_note_off(0, MIDI_PITCH_AS6);

    expect_synth_pitch(0, 4, 0x284);
    expect_synth_volume_any();
    expect_synth_note_on(0);
    __real_midi_note_off(0, MIDI_PITCH_CS4);
}

#define NOTE_PRIORITY_LENGTH 10

void test_midi_drops_note_when_note_priority_stack_full(UNUSED void** state)
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
        expect_value(__wrap_synth_note_on, channel, 0);
        __real_midi_note_on(0, pitch, MAX_MIDI_VOLUME);
    }

    __real_midi_note_on(0, 100, MAX_MIDI_VOLUME);
}

void test_midi_sets_pitch_bend_sensitivity_coarse(UNUSED void** state)
{
    __real_midi_cc(MIDI_CHANNEL_1, CC_RPN_MSB, RPN_PITCH_BEND_SENSITIVITY_MSB);
    __real_midi_cc(MIDI_CHANNEL_1, CC_RPN_LSB, RPN_PITCH_BEND_SENSITIVITY_LSB);
    __real_midi_cc(MIDI_CHANNEL_1, CC_DATA_ENTRY_MSB, 4);

    expect_synth_pitch(YM_CH1, 4, SYNTH_NTSC_C);
    expect_synth_volume_any();
    expect_synth_note_on(YM_CH1);
    __real_midi_note_on(MIDI_CHANNEL_1, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_pitch(YM_CH1, 4, SYNTH_NTSC_E);
    __real_midi_pitch_bend(MIDI_CHANNEL_1, MIDI_PITCH_BEND_MAX);
}

void test_midi_sets_pitch_bend_sensitivity_fine(UNUSED void** state)
{
    __real_midi_cc(MIDI_CHANNEL_1, CC_RPN_MSB, RPN_PITCH_BEND_SENSITIVITY_MSB);
    __real_midi_cc(MIDI_CHANNEL_1, CC_RPN_LSB, RPN_PITCH_BEND_SENSITIVITY_LSB);
    __real_midi_cc(MIDI_CHANNEL_1, CC_DATA_ENTRY_MSB, 4);
    __real_midi_cc(MIDI_CHANNEL_1, CC_DATA_ENTRY_LSB, 25);

    expect_synth_pitch(YM_CH1, 4, SYNTH_NTSC_C);
    expect_synth_volume_any();
    expect_synth_note_on(YM_CH1);
    __real_midi_note_on(MIDI_CHANNEL_1, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_pitch(YM_CH1, 4, 822);
    __real_midi_pitch_bend(MIDI_CHANNEL_1, MIDI_PITCH_BEND_MAX);
}

void test_midi_ignores_further_data_entry_messages_after_null_rpn(UNUSED void** state)
{
    __real_midi_cc(MIDI_CHANNEL_1, CC_RPN_MSB, RPN_PITCH_BEND_SENSITIVITY_MSB);
    __real_midi_cc(MIDI_CHANNEL_1, CC_RPN_LSB, RPN_PITCH_BEND_SENSITIVITY_LSB);
    __real_midi_cc(MIDI_CHANNEL_1, CC_DATA_ENTRY_MSB, 2);
    __real_midi_cc(MIDI_CHANNEL_1, CC_RPN_MSB, NULL_RPN_MSB);
    __real_midi_cc(MIDI_CHANNEL_1, CC_RPN_LSB, NULL_RPN_LSB);
    __real_midi_cc(MIDI_CHANNEL_1, CC_DATA_ENTRY_MSB, 4);

    expect_synth_pitch(YM_CH1, 4, SYNTH_NTSC_C);
    expect_synth_volume_any();
    expect_synth_note_on(YM_CH1);
    __real_midi_note_on(MIDI_CHANNEL_1, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_pitch(YM_CH1, 4, SYNTH_NTSC_D);
    __real_midi_pitch_bend(MIDI_CHANNEL_1, MIDI_PITCH_BEND_MAX);
}

void test_midi_sets_pitch_bend_sensitivity_with_odd_ordering(UNUSED void** state)
{
    __real_midi_cc(MIDI_CHANNEL_1, CC_RPN_LSB, 0x44);
    __real_midi_cc(MIDI_CHANNEL_1, CC_RPN_LSB, RPN_PITCH_BEND_SENSITIVITY_LSB);
    __real_midi_cc(MIDI_CHANNEL_1, CC_RPN_MSB, 0x55);
    __real_midi_cc(MIDI_CHANNEL_1, CC_RPN_MSB, RPN_PITCH_BEND_SENSITIVITY_MSB);
    __real_midi_cc(MIDI_CHANNEL_1, CC_DATA_ENTRY_LSB, 0);
    __real_midi_cc(MIDI_CHANNEL_1, CC_DATA_ENTRY_MSB, 2);
    __real_midi_cc(MIDI_CHANNEL_1, CC_DATA_ENTRY_MSB, 4);

    expect_synth_pitch(YM_CH1, 4, SYNTH_NTSC_C);
    expect_synth_volume_any();
    expect_synth_note_on(YM_CH1);
    __real_midi_note_on(MIDI_CHANNEL_1, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_pitch(YM_CH1, 4, SYNTH_NTSC_E);
    __real_midi_pitch_bend(MIDI_CHANNEL_1, MIDI_PITCH_BEND_MAX);
}
