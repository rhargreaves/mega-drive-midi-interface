#include "cmocka_inc.h"

#include "synth.h"
#include "test_midi.h"
#include <stdbool.h>

extern void __real_synth_init(const FmChannel* defaultPreset);
extern void __real_synth_noteOn(u8 channel);
extern void __real_synth_noteOff(u8 channel);
extern void __real_synth_enableLfo(u8 enable);
extern void __real_synth_globalLfoFrequency(u8 freq);
extern void __real_synth_pitch(u8 channel, u8 octave, u16 freqNumber);
extern void __real_synth_stereo(u8 channel, u8 stereo);
extern void __real_synth_ams(u8 channel, u8 ams);
extern void __real_synth_fms(u8 channel, u8 fms);
extern void __real_synth_algorithm(u8 channel, u8 algorithm);
extern void __real_synth_feedback(u8 channel, u8 feedback);
extern void __real_synth_operatorTotalLevel(u8 channel, u8 op, u8 totalLevel);
extern void __real_synth_operatorMultiple(u8 channel, u8 op, u8 multiple);
extern void __real_synth_operatorDetune(u8 channel, u8 op, u8 detune);
extern void __real_synth_operatorRateScaling(u8 channel, u8 op, u8 rateScaling);
extern void __real_synth_operatorAttackRate(u8 channel, u8 op, u8 attackRate);
extern void __real_synth_operatorFirstDecayRate(
    u8 channel, u8 op, u8 firstDecayRate);
extern void __real_synth_operatorSecondDecayRate(
    u8 channel, u8 op, u8 secondDecayRate);
extern void __real_synth_operatorSecondaryAmplitude(
    u8 channel, u8 op, u8 secondaryAmplitude);
extern void __real_synth_operatorAmplitudeModulation(
    u8 channel, u8 op, u8 amplitudeModulation);
extern void __real_synth_operatorReleaseRate(u8 channel, u8 op, u8 releaseRate);
extern void __real_synth_operatorSsgEg(u8 channel, u8 op, u8 ssgEg);
extern void __real_synth_pitchBend(u8 channel, u16 bend);
extern void __real_synth_preset(u8 channel, const FmChannel* preset);
extern void __real_synth_volume(u8 channel, u8 volume);
extern const FmChannel* __real_synth_channelParameters(u8 channel);
extern const Global* __real_synth_globalParameters();
extern void __real_synth_setSpecialMode(bool enable);
extern void __real_synth_specialModePitch(u8 op, u8 octave, u16 freqNumber);
extern void __real_synth_specialModeVolume(u8 op, u8 volume);
extern void __real_synth_enableDac(bool enable);
extern void __real_synth_directWriteYm2612(u8 part, u8 reg, u8 data);

static bool updated = false;
static u8 lastChan = -1;
static ParameterUpdated lastParameterUpdated = -1;

static void set_initial_registers()
{
    expect_value(__wrap_Z80_loadDriver, driver, 1);
    expect_value(__wrap_Z80_loadDriver, waitReady, true);
    expect_value(__wrap_Z80_requestBus, wait, TRUE);

    const u16 count = 188;
    for (u16 i = 0; i < count; i++) {
        expect_value(__wrap_Z80_getAndRequestBus, wait, TRUE);
        will_return(__wrap_Z80_getAndRequestBus, true);
        expect_any(__wrap_YM2612_writeReg, part);
        expect_any(__wrap_YM2612_writeReg, reg);
        expect_any(__wrap_YM2612_writeReg, data);
    }

    expect_function_call(__wrap_Z80_releaseBus);

    const FmChannel M_BANK_0_INST_0_GRANDPIANO = { 0, 0, 3, 0, 0, 0, 0,
        { { 1, 0, 26, 1, 7, 0, 7, 4, 1, 35, 0 },
            { 4, 6, 24, 1, 9, 0, 6, 9, 7, 35, 0 },
            { 2, 7, 31, 3, 23, 0, 9, 15, 1, 35, 0 },
            { 1, 3, 27, 2, 4, 0, 10, 4, 6, 35, 0 } } };
    __real_synth_init(&M_BANK_0_INST_0_GRANDPIANO);
}

static int test_synth_setup(UNUSED void** state)
{
    updated = false;
    set_initial_registers();
    return 0;
}

static void test_synth_init_sets_initial_registers(UNUSED void** state)
{
    set_initial_registers();
}

static void test_synth_sets_note_on_fm_reg_chan_0_to_2(UNUSED void** state)
{
    for (u8 chan = 0; chan < 3; chan++) {
        expect_ym2612_write_reg(0, 0x28, 0xF0 + chan);
        __real_synth_noteOn(chan);
    }
}

static void test_synth_sets_note_on_fm_reg_chan_3_to_5(UNUSED void** state)
{
    for (u8 chan = 3; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_reg(0, 0x28, 0xF1 + chan);
        __real_synth_noteOn(chan);
    }
}

static void test_synth_sets_note_off_fm_reg_chan_0_to_2(UNUSED void** state)
{
    for (u8 chan = 0; chan < 3; chan++) {
        expect_ym2612_write_reg(0, 0x28, chan);
        __real_synth_noteOff(chan);
    }
}

static void test_synth_sets_note_off_fm_reg_chan_3_to_5(UNUSED void** state)
{
    for (u8 chan = 3; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_reg(0, 0x28, 1 + chan);
        __real_synth_noteOff(chan);
    }
}

static void test_synth_sets_octave_and_freq_reg_chan(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(chan, 0xA4, 0x22);
        expect_ym2612_write_channel(chan, 0xA0, 0x84);
        __real_synth_pitch(chan, 4, SYNTH_NTSC_C);
    }
}

static void test_synth_sets_stereo_ams_and_freq(UNUSED void** state)
{
    const u8 ams = 1;
    const u8 fms = 1;
    const u8 stereo = 1;
    const u8 baseReg = 0xB4;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel_any_data(chan, baseReg);
        __real_synth_stereo(chan, stereo);
        expect_ym2612_write_channel_any_data(chan, baseReg);
        __real_synth_ams(chan, ams);
        expect_ym2612_write_channel(
            chan, baseReg, (stereo << 6) + (ams << 4) + fms);
        __real_synth_fms(chan, fms);
    }
}

static void test_synth_sets_algorithm(UNUSED void** state)
{
    const u8 defaultFeedback = 0;
    const u8 algorithm = 1;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(
            chan, 0xB0, (defaultFeedback << 3) + algorithm);
        __real_synth_algorithm(chan, algorithm);
    }
}

static void test_synth_sets_feedback(UNUSED void** state)
{
    const u8 defaultAlgorithm = 0;
    const u8 feedback = 1;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(
            chan, 0xB0, (feedback << 3) + defaultAlgorithm);
        __real_synth_feedback(chan, feedback);
    }
}

static void test_synth_sets_feedback_and_algorithm(UNUSED void** state)
{
    const u8 baseReg = 0xB0;
    const u8 defaultFeedback = 0;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        u8 feedback = 1;
        u8 algorithm = 1;

        expect_ym2612_write_channel(
            chan, baseReg, (defaultFeedback << 3) + algorithm);
        __real_synth_algorithm(chan, feedback);
        expect_ym2612_write_channel(chan, baseReg, (feedback << 3) + algorithm);
        __real_synth_feedback(chan, feedback);

        feedback++;
        algorithm++;
    }
}

static void test_synth_does_not_reset_operator_level_if_equal(
    UNUSED void** state)
{
    const u8 baseReg = 0x40;
    const u8 totalLevel = 50;
    u8 chan = 0;
    u8 op = 0;

    expect_ym2612_write_operator(chan, op, baseReg, totalLevel);
    __real_synth_operatorTotalLevel(chan, op, totalLevel);
    __real_synth_operatorTotalLevel(chan, op, totalLevel);
}

static void test_synth_sets_operator_total_level(UNUSED void** state)
{
    const u8 baseReg = 0x40;
    const u8 totalLevel = 50;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(chan, op, baseReg, totalLevel);
            __real_synth_operatorTotalLevel(chan, op, totalLevel);
        }
    }
}

static void test_synth_sets_operator_multiple_and_detune(UNUSED void** state)
{
    const u8 baseReg = 0x30;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        u8 multiple = 0;
        u8 detune = 0;
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator_any_data(chan, op, baseReg);
            __real_synth_operatorMultiple(chan, op, multiple);
            expect_ym2612_write_operator(
                chan, op, baseReg, (detune << 4) | multiple);
            __real_synth_operatorDetune(chan, op, detune);
            multiple++;
            detune++;
        }
    }
}

static void test_synth_sets_operator_attack_rate_and_rate_scaling(
    UNUSED void** state)
{
    const u8 baseReg = 0x50;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        u8 attackRate = 0;
        u8 rateScaling = 0;
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator_any_data(chan, op, baseReg);
            __real_synth_operatorAttackRate(chan, op, attackRate);
            expect_ym2612_write_operator(
                chan, op, baseReg, attackRate | (rateScaling << 6));
            __real_synth_operatorRateScaling(chan, op, rateScaling);
            attackRate++;
            rateScaling++;
        }
    }
}

static void test_synth_sets_operator_second_decay_rate(UNUSED void** state)
{
    const u8 baseReg = 0x70;
    u8 secondDecayRate = 16;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(chan, op, baseReg, secondDecayRate);
            __real_synth_operatorSecondDecayRate(chan, op, secondDecayRate);
        }
    }
}

static void test_synth_sets_operator_release_rate_and_secondary_amplitude(
    UNUSED void** state)
{
    const u8 baseReg = 0x80;
    u8 secondaryAmplitude = 15;
    u8 releaseRate = 4;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator_any_data(chan, op, baseReg);
            __real_synth_operatorSecondaryAmplitude(
                chan, op, secondaryAmplitude);
            expect_ym2612_write_operator(
                chan, op, baseReg, releaseRate + (secondaryAmplitude << 4));
            __real_synth_operatorReleaseRate(chan, op, releaseRate);
        }
    }
}

static void test_synth_sets_operator_amplitude_modulation_and_first_decay_rate(
    UNUSED void** state)
{
    const u8 baseReg = 0x60;
    u8 amplitudeModulation = 1;
    u8 firstDecayRate = 16;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator_any_data(chan, op, baseReg);
            __real_synth_operatorFirstDecayRate(chan, op, firstDecayRate);
            expect_ym2612_write_operator(
                chan, op, baseReg, firstDecayRate + (amplitudeModulation << 7));
            __real_synth_operatorAmplitudeModulation(
                chan, op, amplitudeModulation);
        }
    }
}

static void test_synth_sets_operator_ssg_eg(UNUSED void** state)
{
    const u8 baseReg = 0x90;
    const u8 ssgEg = 11;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(chan, op, baseReg, ssgEg);
            __real_synth_operatorSsgEg(chan, op, ssgEg);
        }
    }
}

static void test_synth_sets_global_LFO_enable_and_frequency(UNUSED void** state)
{
    const u8 baseReg = 0x22;
    expect_ym2612_write_reg_any_data(0, baseReg);
    __real_synth_enableLfo(1);
    expect_ym2612_write_reg(0, baseReg, (1 << 3) | 1);
    __real_synth_globalLfoFrequency(1);
}

static void test_synth_sets_busy_indicators(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan += 2) {
        expect_ym2612_write_reg_any_data(0, 0x28);
        __real_synth_noteOn(chan);
    }
    u8 busy = synth_busy();
    assert_int_equal(busy, 0b00010101);
}

static void test_synth_sets_preset(UNUSED void** state)
{
    const u8 chan = 0;

    const FmChannel M_BANK_0_INST_7_CLAVINET = { 1, 7, 3, 0, 0, 0, 0,
        { { 1, 1, 31, 2, 0, 1, 0, 1, 6, 28, 0 },
            { 1, 3, 31, 2, 0, 0, 0, 1, 7, 33, 0 },
            { 1, 5, 31, 3, 0, 1, 0, 0, 2, 30, 0 },
            { 1, 7, 31, 0, 6, 0, 4, 6, 7, 6, 0 } } };

    expect_ym2612_write_channel_any_data(chan, 0xB0);
    expect_ym2612_write_channel_any_data(chan, 0xB4);
    expect_ym2612_write_channel_any_data(chan, 0x30);
    expect_ym2612_write_channel_any_data(chan, 0x50);
    expect_ym2612_write_channel_any_data(chan, 0x60);
    expect_ym2612_write_channel_any_data(chan, 0x70);
    expect_ym2612_write_channel_any_data(chan, 0x80);
    expect_ym2612_write_channel_any_data(chan, 0x40);
    expect_ym2612_write_channel_any_data(chan, 0x90);
    expect_ym2612_write_channel_any_data(chan, 0x38);
    expect_ym2612_write_channel_any_data(chan, 0x58);
    expect_ym2612_write_channel_any_data(chan, 0x68);
    expect_ym2612_write_channel_any_data(chan, 0x78);
    expect_ym2612_write_channel_any_data(chan, 0x88);
    expect_ym2612_write_channel_any_data(chan, 0x48);
    expect_ym2612_write_channel_any_data(chan, 0x98);
    expect_ym2612_write_channel_any_data(chan, 0x34);
    expect_ym2612_write_channel_any_data(chan, 0x54);
    expect_ym2612_write_channel_any_data(chan, 0x64);
    expect_ym2612_write_channel_any_data(chan, 0x74);
    expect_ym2612_write_channel_any_data(chan, 0x84);
    expect_ym2612_write_channel_any_data(chan, 0x44);
    expect_ym2612_write_channel_any_data(chan, 0x94);
    expect_ym2612_write_channel_any_data(chan, 0x3C);
    expect_ym2612_write_channel_any_data(chan, 0x5C);
    expect_ym2612_write_channel_any_data(chan, 0x6C);
    expect_ym2612_write_channel_any_data(chan, 0x7C);
    expect_ym2612_write_channel_any_data(chan, 0x8C);
    expect_ym2612_write_channel_any_data(chan, 0x4C);
    expect_ym2612_write_channel_any_data(chan, 0x9C);

    __real_synth_preset(chan, &M_BANK_0_INST_7_CLAVINET);
}

static void test_synth_applies_volume_modifier_to_output_operators_algorithm_7(
    UNUSED void** state)
{
    const u8 algorithm = 7;
    const u8 totalLevelReg = 0x40;
    const u8 algorithmReg = 0xB0;
    const u8 loudestTotalLevel = 0;
    const u8 loudestVolume = 0x7F;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel_any_data(chan, algorithmReg);
        __real_synth_algorithm(chan, algorithm);

        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(
                chan, op, totalLevelReg, loudestTotalLevel);
        }
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            __real_synth_operatorTotalLevel(chan, op, loudestTotalLevel);
        }

        const u8 expectedTotalLevel = 0xb;
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(
                chan, op, totalLevelReg, expectedTotalLevel);
        }
        __real_synth_volume(chan, loudestVolume / 2);

        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(
                chan, op, totalLevelReg, loudestTotalLevel);
        }
        __real_synth_volume(chan, loudestVolume);
    }
}

static void test_synth_does_not_apply_volume_if_equal(UNUSED void** state)
{
    const u8 algorithm = 7;
    const u8 totalLevelReg = 0x40;
    const u8 algorithmReg = 0xB0;
    const u8 loudestTotalLevel = 0;
    const u8 loudestVolume = 0x7F;
    u8 chan = 0;
    expect_ym2612_write_channel_any_data(chan, algorithmReg);
    __real_synth_algorithm(chan, algorithm);

    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        expect_ym2612_write_operator(
            chan, op, totalLevelReg, loudestTotalLevel);
    }
    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        __real_synth_operatorTotalLevel(chan, op, loudestTotalLevel);
    }

    const u8 expectedTotalLevel = 0xb;
    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        expect_ym2612_write_operator(
            chan, op, totalLevelReg, expectedTotalLevel);
    }
    __real_synth_volume(chan, loudestVolume / 2);

    __real_synth_volume(chan, loudestVolume / 2);
}

static void
test_synth_applies_volume_modifier_to_output_operators_algorithm_7_quieter(
    UNUSED void** state)
{
    const u8 algorithm = 7;
    const u8 totalLevelReg = 0x40;
    const u8 algorithmReg = 0xB0;
    const u8 loudestTotalLevel = 0;
    const u8 loudestVolume = 0x7F;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel_any_data(chan, algorithmReg);
        __real_synth_algorithm(chan, algorithm);

        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(
                chan, op, totalLevelReg, loudestTotalLevel);
        }
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            __real_synth_operatorTotalLevel(chan, op, loudestTotalLevel);
        }

        const u8 expectedTotalLevel = 0x26;
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(
                chan, op, totalLevelReg, expectedTotalLevel);
        }
        __real_synth_volume(chan, loudestVolume / 4);
    }
}

static void
test_synth_applies_volume_modifier_to_output_operators_algorithms_0_to_3(
    UNUSED void** state)
{
    const u8 totalLevelReg = 0x40;
    const u8 algorithmReg = 0xB0;
    const u8 loudestVolume = 0x7F;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 algorithm = 0; algorithm < 4; algorithm++) {
            print_message("chan %d, alg %d setup\n", chan, algorithm);
            expect_ym2612_write_channel(chan, algorithmReg, algorithm);
            __real_synth_algorithm(chan, algorithm);

            if (algorithm == 0) {
                /* Operator values are not re-applied for algorithms 1-3 due
                to unnecessary YM2612 writing optimisation */
                expect_ym2612_write_operator(chan, 0, totalLevelReg, 0x23);
                expect_ym2612_write_operator(chan, 1, totalLevelReg, 0x23);
                expect_ym2612_write_operator(chan, 2, totalLevelReg, 0x23);
                expect_ym2612_write_operator(chan, 3, totalLevelReg, 0x3f);
            }
            __real_synth_volume(chan, loudestVolume / 4);
        }
    }
}

static void test_synth_applies_volume_modifier_to_output_operators_algorithm_4(
    UNUSED void** state)
{
    const u8 totalLevelReg = 0x40;
    const u8 algorithmReg = 0xB0;
    const u8 loudestVolume = 0x7F;
    const u8 algorithm = 4;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(chan, algorithmReg, algorithm);
        __real_synth_algorithm(chan, algorithm);

        expect_ym2612_write_operator(chan, 0, totalLevelReg, 0x23);
        expect_ym2612_write_operator(chan, 1, totalLevelReg, 0x3f);
        expect_ym2612_write_operator(chan, 2, totalLevelReg, 0x23);
        expect_ym2612_write_operator(chan, 3, totalLevelReg, 0x3f);
        __real_synth_volume(chan, loudestVolume / 4);
    }
}

static void
test_synth_applies_volume_modifier_to_output_operators_algorithms_5_and_6(
    UNUSED void** state)
{
    const u8 totalLevelReg = 0x40;
    const u8 algorithmReg = 0xB0;
    const u8 loudestVolume = 0x7F;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 algorithm = 5; algorithm < 7; algorithm++) {
            expect_ym2612_write_channel(chan, algorithmReg, algorithm);
            __real_synth_algorithm(chan, algorithm);

            if (algorithm == 5) {
                /* Operator values are not re-applied for algorithms 1-3 due
                to unnecessary YM2612 writing optimisation */
                expect_ym2612_write_operator(chan, 0, totalLevelReg, 0x23);
                expect_ym2612_write_operator(chan, 1, totalLevelReg, 0x3f);
                expect_ym2612_write_operator(chan, 2, totalLevelReg, 0x3f);
                expect_ym2612_write_operator(chan, 3, totalLevelReg, 0x3f);
            }
            __real_synth_volume(chan, loudestVolume / 4);
        }
    }
}

static void test_synth_exposes_fm_channel_parameters(UNUSED void** state)
{
    const FmChannel* chan = __real_synth_channelParameters(0);

    assert_int_equal(chan->stereo, STEREO_MODE_CENTRE);
}

static void test_synth_exposes_global_parameters(UNUSED void** state)
{
    const Global* global = __real_synth_globalParameters();

    assert_int_equal(global->lfoFrequency, 1);
}

static void updateCallback(u8 chan, ParameterUpdated parameterUpdated)
{
    updated = true;
    lastChan = chan;
    lastParameterUpdated = parameterUpdated;
}

static void test_synth_calls_callback_when_parameter_changes(
    UNUSED void** state)
{
    synth_setParameterUpdateCallback(&updateCallback);

    const u8 defaultFeedback = 0;
    const u8 algorithm = 1;
    u8 fmChan = 1;
    expect_ym2612_write_channel(
        fmChan, 0xB0, (defaultFeedback << 3) + algorithm);
    __real_synth_algorithm(fmChan, algorithm);

    assert_true(updated);
    assert_int_equal(lastChan, fmChan);
    assert_int_equal(lastParameterUpdated, Channel);
}

static void test_synth_calls_callback_when_lfo_freq_changes(UNUSED void** state)
{
    synth_setParameterUpdateCallback(&updateCallback);

    expect_ym2612_write_reg_any_data(0, 0x22);
    __real_synth_globalLfoFrequency(1);

    assert_true(updated);
    assert_int_equal(lastParameterUpdated, Lfo);
}

static void test_synth_calls_callback_when_lfo_enable_changes(
    UNUSED void** state)
{
    synth_setParameterUpdateCallback(&updateCallback);

    expect_ym2612_write_reg_any_data(0, 0x22);
    __real_synth_enableLfo(0);

    assert_true(updated);
    assert_int_equal(lastParameterUpdated, Lfo);
}

static void test_synth_calls_callback_when_special_mode_changes(
    UNUSED void** state)
{
    synth_setParameterUpdateCallback(&updateCallback);

    expect_ym2612_write_reg_any_data(0, 0x27);
    __real_synth_setSpecialMode(true);

    assert_true(updated);
    assert_int_equal(lastParameterUpdated, SpecialMode);
}

static void test_synth_enables_ch3_special_mode(UNUSED void** state)
{
    expect_ym2612_write_reg(0, 0x27, 0x40);
    __real_synth_setSpecialMode(true);
}

static void test_synth_disables_ch3_special_mode(UNUSED void** state)
{
    expect_ym2612_write_reg(0, 0x27, 0);
    __real_synth_setSpecialMode(false);
}

static void test_synth_sets_ch3_special_mode_operator_pitches(
    UNUSED void** state)
{
    const u8 upperRegs[] = { 0xAD, 0xAE, 0xAC };
    const u8 lowerRegs[] = { 0xA9, 0xAA, 0xA8 };

    for (u8 op = 0; op < 3; op++) {
        expect_ym2612_write_reg(0, upperRegs[op], 0x22);
        expect_ym2612_write_reg(0, lowerRegs[op], 0x84);

        __real_synth_specialModePitch(op, 4, SYNTH_NTSC_C);
    }
}

static void test_synth_handles_out_of_range_ch3_special_mode_operator(
    UNUSED void** state)
{
    const u8 op = 3; // invalid op
    expect_ym2612_write_reg(0, 0xAD, 0x22); // safely wrap to valid reg
    expect_ym2612_write_reg(0, 0xA9, 0x84);

    __real_synth_specialModePitch(op, 4, SYNTH_NTSC_C);
}

static void synth_sets_ch3_special_mode_op_tl_only_if_output_operator(int alg)
{
    const u8 baseReg = 0x40;
    const u8 chan = CH_SPECIAL_MODE;

    expect_ym2612_write_channel(chan, 0xB0, alg);
    __real_synth_algorithm(chan, alg);

    for (u8 op = 0; op <= 2; op++) {
        // set initial TL
        expect_ym2612_write_operator(chan, op, baseReg, 2);
        __real_synth_operatorTotalLevel(chan, op, 2);

        switch (alg) {
        case 0:
        case 1:
        case 2:
        case 3: {
            // expect nothing to happen
            __real_synth_specialModeVolume(op, 60);
            break;
        }
        case 4: {
            if (op == 0 || op == 2) {
                // expect nothing to happen
                __real_synth_specialModeVolume(op, 60);
            } else {
                // set volume
                expect_ym2612_write_operator(chan, op, baseReg, 14);
                __real_synth_specialModeVolume(op, 60);
            }
            break;
        }
        case 5:
        case 6: {
            if (op == 0) {
                // expect nothing to happen
                __real_synth_specialModeVolume(op, 60);
            } else {
                // set volume
                expect_ym2612_write_operator(chan, op, baseReg, 14);
                __real_synth_specialModeVolume(op, 60);
            }
            break;
        }
        case 7: {
            // set volume
            expect_ym2612_write_operator(chan, op, baseReg, 14);
            __real_synth_specialModeVolume(op, 60);
            break;
        }
        }
    }
}

static void
test_synth_sets_ch3_special_mode_op_tl_only_if_output_operator_alg_0(
    UNUSED void** state)
{
    synth_sets_ch3_special_mode_op_tl_only_if_output_operator(0);
}

static void
test_synth_sets_ch3_special_mode_op_tl_only_if_output_operator_alg_1(
    UNUSED void** state)
{
    synth_sets_ch3_special_mode_op_tl_only_if_output_operator(1);
}

static void
test_synth_sets_ch3_special_mode_op_tl_only_if_output_operator_alg_2(
    UNUSED void** state)
{
    synth_sets_ch3_special_mode_op_tl_only_if_output_operator(2);
}

static void
test_synth_sets_ch3_special_mode_op_tl_only_if_output_operator_alg_3(
    UNUSED void** state)
{
    synth_sets_ch3_special_mode_op_tl_only_if_output_operator(3);
}

static void
test_synth_sets_ch3_special_mode_op_tl_only_if_output_operator_alg_4(
    UNUSED void** state)
{
    synth_sets_ch3_special_mode_op_tl_only_if_output_operator(4);
}

static void
test_synth_sets_ch3_special_mode_op_tl_only_if_output_operator_alg_5(
    UNUSED void** state)
{
    synth_sets_ch3_special_mode_op_tl_only_if_output_operator(5);
}

static void
test_synth_sets_ch3_special_mode_op_tl_only_if_output_operator_alg_6(
    UNUSED void** state)
{
    synth_sets_ch3_special_mode_op_tl_only_if_output_operator(6);
}

static void
test_synth_sets_ch3_special_mode_op_tl_only_if_output_operator_alg_7(
    UNUSED void** state)
{
    synth_sets_ch3_special_mode_op_tl_only_if_output_operator(7);
}

static void test_synth_enables_dac(UNUSED void** state)
{
    expect_ym2612_write_reg(0, 0x2B, 0x80);

    __real_synth_enableDac(true);
}

static void test_synth_disables_dac(UNUSED void** state)
{
    expect_ym2612_write_reg(0, 0x2B, 0);

    __real_synth_enableDac(false);
}

static void test_requests_Z80_bus_if_not_already_taken(UNUSED void** state)
{
    expect_value(__wrap_Z80_getAndRequestBus, wait, TRUE);
    will_return(__wrap_Z80_getAndRequestBus, false);

    expect_value(__wrap_YM2612_writeReg, part, 0);
    expect_value(__wrap_YM2612_writeReg, reg, 0x2B);
    expect_value(__wrap_YM2612_writeReg, data, 0);

    expect_function_call(__wrap_Z80_releaseBus);

    __real_synth_directWriteYm2612(0, 0x2B, 0);
}

static void test_does_not_release_Z80_bus_when_taken_prior_to_call(
    UNUSED void** state)
{
    expect_value(__wrap_Z80_getAndRequestBus, wait, TRUE);
    will_return(__wrap_Z80_getAndRequestBus, true);

    expect_value(__wrap_YM2612_writeReg, part, 0);
    expect_value(__wrap_YM2612_writeReg, reg, 0x2B);
    expect_value(__wrap_YM2612_writeReg, data, 0);

    __real_synth_directWriteYm2612(0, 0x2B, 0);
}
