#include "test_synth.h"
#include "synth.h"
#include "mocks/mock_comm.h"
#include "mocks/mock_synth.h"
#include "test_midi.h"
#include "z80_ctrl.h"
#include "mocks/mock_ym2612.h"
#include "ym2612_regs.h"
#include "test_helpers.h"

static bool updated = false;
static u8 lastChan = -1;
static ParameterUpdated lastParameterUpdated = -1;

static void set_initial_registers(void)
{
    expect_value(__wrap_Z80_requestBus, wait, TRUE);

    const u16 count = 188;
    for (u16 i = 0; i < count; i++) {
        expect_value(__wrap_Z80_getAndRequestBus, wait, TRUE);
        will_return(__wrap_Z80_getAndRequestBus, true);
        expect_any(__wrap_YM2612_writeReg, part);
        expect_any(__wrap_YM2612_writeReg, reg);
        expect_any(__wrap_YM2612_writeReg, data);
    }

    expect_value(__wrap_YM2612_write, port, 0);
    expect_value(__wrap_YM2612_write, data, YM_DAC_DATA);
    expect_function_call(__wrap_Z80_releaseBus);

    const FmChannel M_BANK_0_INST_0_GRANDPIANO = { 0, 0, 3, 0, 0, 0, 0,
        { { 1, 0, 26, 1, 7, 0, 7, 4, 1, 35, 0 }, { 4, 6, 24, 1, 9, 0, 6, 9, 7, 35, 0 },
            { 2, 7, 31, 3, 23, 0, 9, 15, 1, 35, 0 }, { 1, 3, 27, 2, 4, 0, 10, 4, 6, 35, 0 } } };
    __real_synth_init(&M_BANK_0_INST_0_GRANDPIANO);
}

static void loads_pcm_driver(void)
{
    expect_value(__wrap_Z80_loadDriver, driver, Z80_DRIVER_PCM);
    expect_value(__wrap_Z80_loadDriver, waitReady, true);
}

int test_synth_setup(UNUSED void** state)
{
    updated = false;
    loads_pcm_driver();
    set_initial_registers();
    return 0;
}

void test_synth_init_sets_initial_registers(UNUSED void** state)
{
    loads_pcm_driver();
    set_initial_registers();
}

void test_synth_sets_note_on_fm_reg_chan_0_to_2(UNUSED void** state)
{
    for (u8 chan = 0; chan < 3; chan++) {
        expect_ym2612_note_on(chan);
        __real_synth_noteOn(chan);
    }
}

void test_synth_sets_note_on_fm_reg_chan_3_to_5(UNUSED void** state)
{
    for (u8 chan = 3; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_note_on(chan);
        __real_synth_noteOn(chan);
    }
}

void test_synth_sets_note_off_fm_reg_chan_0_to_2(UNUSED void** state)
{
    for (u8 chan = 0; chan < 3; chan++) {
        expect_ym2612_note_off(chan);
        __real_synth_noteOff(chan);
    }
}

void test_synth_sets_note_off_fm_reg_chan_3_to_5(UNUSED void** state)
{
    for (u8 chan = 3; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_note_off(chan);
        __real_synth_noteOff(chan);
    }
}

void test_synth_sets_octave_and_freq_reg_chan(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(chan, YM_BASE_FREQ_MSB_BLK, 0x22);
        expect_ym2612_write_channel(chan, YM_BASE_FREQ_LSB, 0x84);
        __real_synth_pitch(chan, 4, SYNTH_NTSC_C);
    }
}

void test_synth_sets_stereo_ams_and_freq(UNUSED void** state)
{
    const u8 ams = 1;
    const u8 fms = 1;
    const u8 stereo = 1;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel_any_data(chan, YM_BASE_STEREO_AMS_PMS);
        __real_synth_stereo(chan, stereo);
        expect_ym2612_write_channel_any_data(chan, YM_BASE_STEREO_AMS_PMS);
        __real_synth_ams(chan, ams);
        expect_ym2612_write_channel(chan, YM_BASE_STEREO_AMS_PMS, (stereo << 6) + (ams << 4) + fms);
        __real_synth_fms(chan, fms);
    }
}

void test_synth_sets_algorithm(UNUSED void** state)
{
    const u8 defaultFeedback = 0;
    const u8 algorithm = 1;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(
            chan, YM_BASE_ALGORITHM_FEEDBACK, (defaultFeedback << 3) + algorithm);
        __real_synth_algorithm(chan, algorithm);
    }
}

void test_synth_sets_feedback(UNUSED void** state)
{
    const u8 defaultAlgorithm = 0;
    const u8 feedback = 1;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(
            chan, YM_BASE_ALGORITHM_FEEDBACK, (feedback << 3) + defaultAlgorithm);
        __real_synth_feedback(chan, feedback);
    }
}

void test_synth_sets_feedback_and_algorithm(UNUSED void** state)
{
    const u8 defaultFeedback = 0;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        u8 feedback = 1;
        u8 algorithm = 1;

        expect_ym2612_write_channel(
            chan, YM_BASE_ALGORITHM_FEEDBACK, (defaultFeedback << 3) + algorithm);
        __real_synth_algorithm(chan, feedback);
        expect_ym2612_write_channel(chan, YM_BASE_ALGORITHM_FEEDBACK, (feedback << 3) + algorithm);
        __real_synth_feedback(chan, feedback);

        feedback++;
        algorithm++;
    }
}

void test_synth_does_not_reset_operator_level_if_equal(UNUSED void** state)
{
    const u8 totalLevel = 50;
    u8 chan = 0;
    u8 op = 0;

    expect_ym2612_write_operator(chan, op, YM_BASE_TOTAL_LEVEL, totalLevel);
    __real_synth_operatorTotalLevel(chan, op, totalLevel);
    __real_synth_operatorTotalLevel(chan, op, totalLevel);
}

void test_synth_sets_operator_total_level(UNUSED void** state)
{
    const u8 totalLevel = 50;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(chan, op, YM_BASE_TOTAL_LEVEL, totalLevel);
            __real_synth_operatorTotalLevel(chan, op, totalLevel);
        }
    }
}

void test_synth_sets_operator_multiple_and_detune(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        u8 multiple = 0;
        u8 detune = 0;
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator_any_data(chan, op, YM_BASE_MULTIPLE_DETUNE);
            __real_synth_operatorMultiple(chan, op, multiple);
            expect_ym2612_write_operator(
                chan, op, YM_BASE_MULTIPLE_DETUNE, (detune << 4) | multiple);
            __real_synth_operatorDetune(chan, op, detune);
            multiple++;
            detune++;
        }
    }
}

void test_synth_sets_operator_attack_rate_and_rate_scaling(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        u8 attackRate = 0;
        u8 rateScaling = 0;
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator_any_data(chan, op, YM_BASE_ATTACK_RATE_SCALING_RATE);
            __real_synth_operatorAttackRate(chan, op, attackRate);
            expect_ym2612_write_operator(
                chan, op, YM_BASE_ATTACK_RATE_SCALING_RATE, attackRate | (rateScaling << 6));
            __real_synth_operatorRateScaling(chan, op, rateScaling);
            attackRate++;
            rateScaling++;
        }
    }
}

void test_synth_sets_operator_sustain_rate(UNUSED void** state)
{
    u8 sustainRate = 16;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(chan, op, YM_BASE_SUSTAIN_RATE, sustainRate);
            __real_synth_operatorSustainRate(chan, op, sustainRate);
        }
    }
}

void test_synth_sets_operator_release_rate_and_sustain_level(UNUSED void** state)
{
    u8 sustainLevel = 15;
    u8 releaseRate = 4;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator_any_data(chan, op, YM_BASE_RELEASE_RATE_SUSTAIN_LEVEL);
            __real_synth_operatorSustainLevel(chan, op, sustainLevel);
            expect_ym2612_write_operator(
                chan, op, YM_BASE_RELEASE_RATE_SUSTAIN_LEVEL, releaseRate + (sustainLevel << 4));
            __real_synth_operatorReleaseRate(chan, op, releaseRate);
        }
    }
}

void test_synth_sets_operator_amplitude_modulation_and_decay_rate(UNUSED void** state)
{
    u8 amplitudeModulation = 1;
    u8 decayRate = 16;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator_any_data(chan, op, YM_BASE_DECAY_RATE_AM_ENABLE);
            __real_synth_operatorDecayRate(chan, op, decayRate);
            expect_ym2612_write_operator(
                chan, op, YM_BASE_DECAY_RATE_AM_ENABLE, decayRate + (amplitudeModulation << 7));
            __real_synth_operatorAmplitudeModulation(chan, op, amplitudeModulation);
        }
    }
}

void test_synth_sets_operator_ssg_eg(UNUSED void** state)
{
    const u8 ssgEg = 11;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(chan, op, YM_BASE_SSG_EG, ssgEg);
            __real_synth_operatorSsgEg(chan, op, ssgEg);
        }
    }
}

void test_synth_sets_global_LFO_enable_and_frequency(UNUSED void** state)
{
    expect_ym2612_write_reg_any_data(0, YM_LFO_ENABLE);
    __real_synth_enableLfo(1);
    expect_ym2612_write_reg(0, YM_LFO_ENABLE, (1 << 3) | 1);
    __real_synth_globalLfoFrequency(1);
}

void test_synth_sets_busy_indicators(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan += 2) {
        expect_ym2612_write_reg_any_data(0, YM_KEY_ON_OFF);
        __real_synth_noteOn(chan);
    }
    u8 busy = synth_busy();
    assert_int_equal(busy, 0b00010101);
}

void test_synth_sets_preset(UNUSED void** state)
{
    const u8 chan = 0;

    const FmChannel M_BANK_0_INST_7_CLAVINET = { 1, 7, 3, 0, 0, 0, 0,
        { { 1, 1, 31, 2, 0, 1, 0, 1, 6, 28, 0 }, { 1, 3, 31, 2, 0, 0, 0, 1, 7, 33, 0 },
            { 1, 5, 31, 3, 0, 1, 0, 0, 2, 30, 0 }, { 1, 7, 31, 0, 6, 0, 4, 6, 7, 6, 0 } } };

    expect_ym2612_write_channel_any_data(chan, YM_BASE_ALGORITHM_FEEDBACK);
    expect_ym2612_write_channel_any_data(chan, YM_BASE_STEREO_AMS_PMS);

    for (u8 op = YM_OP1; op <= YM_OP4; op++) {
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_MULTIPLE_DETUNE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_ATTACK_RATE_SCALING_RATE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_DECAY_RATE_AM_ENABLE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_SUSTAIN_RATE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_RELEASE_RATE_SUSTAIN_LEVEL, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_TOTAL_LEVEL, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_SSG_EG, op));
    }

    __real_synth_preset(chan, &M_BANK_0_INST_7_CLAVINET);
}

void test_synth_sets_preset_retaining_pan(UNUSED void** state)
{
    const u8 chan = 0;

    expect_ym2612_write_channel(chan, YM_BASE_STEREO_AMS_PMS, 0x80);
    __real_synth_stereo(chan, STEREO_MODE_LEFT);

    const FmChannel M_BANK_0_INST_7_CLAVINET = { 1, 7, 3, 0, 0, 0, 0,
        { { 1, 1, 31, 2, 0, 1, 0, 1, 6, 28, 0 }, { 1, 3, 31, 2, 0, 0, 0, 1, 7, 33, 0 },
            { 1, 5, 31, 3, 0, 1, 0, 0, 2, 30, 0 }, { 1, 7, 31, 0, 6, 0, 4, 6, 7, 6, 0 } } };

    expect_ym2612_write_channel_any_data(chan, YM_BASE_ALGORITHM_FEEDBACK);
    expect_ym2612_write_channel(chan, YM_BASE_STEREO_AMS_PMS, 0x80);

    for (u8 op = YM_OP1; op <= YM_OP4; op++) {
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_MULTIPLE_DETUNE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_ATTACK_RATE_SCALING_RATE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_DECAY_RATE_AM_ENABLE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_SUSTAIN_RATE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_RELEASE_RATE_SUSTAIN_LEVEL, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_TOTAL_LEVEL, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_SSG_EG, op));
    }
    __real_synth_preset(chan, &M_BANK_0_INST_7_CLAVINET);
}

void test_synth_applies_volume_modifier_to_output_operators_algorithm_7(UNUSED void** state)
{
    const u8 algorithm = 7;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel_any_data(chan, YM_BASE_ALGORITHM_FEEDBACK);
        __real_synth_algorithm(chan, algorithm);

        expect_ym2612_write_all_operators(chan, YM_BASE_TOTAL_LEVEL, YM_TOTAL_LEVEL_LOUDEST);
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            __real_synth_operatorTotalLevel(chan, op, YM_TOTAL_LEVEL_LOUDEST);
        }

        const u8 expectedTotalLevel = 0xb;
        expect_ym2612_write_all_operators(chan, YM_BASE_TOTAL_LEVEL, expectedTotalLevel);
        __real_synth_volume(chan, SYNTH_VOLUME_MAX / 2);

        expect_ym2612_write_all_operators(chan, YM_BASE_TOTAL_LEVEL, YM_TOTAL_LEVEL_LOUDEST);
        __real_synth_volume(chan, SYNTH_VOLUME_MAX);
    }
}

void test_synth_does_not_apply_volume_if_equal(UNUSED void** state)
{
    const u8 algorithm = 7;
    u8 chan = 0;
    expect_ym2612_write_channel_any_data(chan, YM_BASE_ALGORITHM_FEEDBACK);
    __real_synth_algorithm(chan, algorithm);

    expect_ym2612_write_all_operators(chan, YM_BASE_TOTAL_LEVEL, YM_TOTAL_LEVEL_LOUDEST);
    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        __real_synth_operatorTotalLevel(chan, op, YM_TOTAL_LEVEL_LOUDEST);
    }

    const u8 expectedTotalLevel = 0xb;
    expect_ym2612_write_all_operators(chan, YM_BASE_TOTAL_LEVEL, expectedTotalLevel);
    __real_synth_volume(chan, SYNTH_VOLUME_MAX / 2);

    __real_synth_volume(chan, SYNTH_VOLUME_MAX / 2);
}

void test_synth_applies_volume_modifier_to_output_operators_algorithm_7_quieter(UNUSED void** state)
{
    const u8 algorithm = 7;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel_any_data(chan, YM_BASE_ALGORITHM_FEEDBACK);
        __real_synth_algorithm(chan, algorithm);

        expect_ym2612_write_all_operators(chan, YM_BASE_TOTAL_LEVEL, 0);
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            __real_synth_operatorTotalLevel(chan, op, YM_TOTAL_LEVEL_LOUDEST);
        }

        const u8 expectedTotalLevel = 0x26;
        expect_ym2612_write_all_operators(chan, YM_BASE_TOTAL_LEVEL, expectedTotalLevel);
        __real_synth_volume(chan, SYNTH_VOLUME_MAX / 4);
    }
}

void test_synth_applies_volume_modifier_to_output_operators_algorithms_0_to_3(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 algorithm = 0; algorithm < 4; algorithm++) {
            print_message("chan %d, alg %d setup\n", chan, algorithm);
            expect_ym2612_write_channel(chan, YM_BASE_ALGORITHM_FEEDBACK, algorithm);
            __real_synth_algorithm(chan, algorithm);

            if (algorithm == 0) {
                /* Operator values are not re-applied for algorithms 1-3 due
                to unnecessary YM2612 writing optimisation */
                expect_ym2612_write_operator(chan, 0, YM_BASE_TOTAL_LEVEL, 0x23);
                expect_ym2612_write_operator(chan, 1, YM_BASE_TOTAL_LEVEL, 0x23);
                expect_ym2612_write_operator(chan, 2, YM_BASE_TOTAL_LEVEL, 0x23);
                expect_ym2612_write_operator(chan, 3, YM_BASE_TOTAL_LEVEL, 0x3f);
            }
            __real_synth_volume(chan, SYNTH_VOLUME_MAX / 4);
        }
    }
}

void test_synth_applies_volume_modifier_to_output_operators_algorithm_4(UNUSED void** state)
{
    const u8 algorithm = 4;

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(chan, YM_BASE_ALGORITHM_FEEDBACK, algorithm);
        __real_synth_algorithm(chan, algorithm);

        expect_ym2612_write_operator(chan, 0, YM_BASE_TOTAL_LEVEL, 0x23);
        expect_ym2612_write_operator(chan, 1, YM_BASE_TOTAL_LEVEL, 0x3f);
        expect_ym2612_write_operator(chan, 2, YM_BASE_TOTAL_LEVEL, 0x23);
        expect_ym2612_write_operator(chan, 3, YM_BASE_TOTAL_LEVEL, 0x3f);
        __real_synth_volume(chan, SYNTH_VOLUME_MAX / 4);
    }
}

void test_synth_applies_volume_modifier_to_output_operators_algorithms_5_and_6(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 algorithm = 5; algorithm < 7; algorithm++) {
            expect_ym2612_write_channel(chan, YM_BASE_ALGORITHM_FEEDBACK, algorithm);
            __real_synth_algorithm(chan, algorithm);

            if (algorithm == 5) {
                /* Operator values are not re-applied for algorithms 1-3 due
                to unnecessary YM2612 writing optimisation */
                expect_ym2612_write_operator(chan, 0, YM_BASE_TOTAL_LEVEL, 0x23);
                expect_ym2612_write_operator(chan, 1, YM_BASE_TOTAL_LEVEL, 0x3f);
                expect_ym2612_write_operator(chan, 2, YM_BASE_TOTAL_LEVEL, 0x3f);
                expect_ym2612_write_operator(chan, 3, YM_BASE_TOTAL_LEVEL, 0x3f);
            }
            __real_synth_volume(chan, SYNTH_VOLUME_MAX / 4);
        }
    }
}

void test_synth_exposes_fm_channel_parameters(UNUSED void** state)
{
    const FmChannel* chan = __real_synth_channelParameters(0);

    assert_int_equal(chan->stereo, STEREO_MODE_CENTRE);
}

void test_synth_exposes_global_parameters(UNUSED void** state)
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

void test_synth_calls_callback_when_parameter_changes(UNUSED void** state)
{
    synth_setParameterUpdateCallback(&updateCallback);

    const u8 defaultFeedback = 0;
    const u8 algorithm = 1;
    u8 fmChan = 1;
    expect_ym2612_write_channel(
        fmChan, YM_BASE_ALGORITHM_FEEDBACK, (defaultFeedback << 3) + algorithm);
    __real_synth_algorithm(fmChan, algorithm);

    assert_true(updated);
    assert_int_equal(lastChan, fmChan);
    assert_int_equal(lastParameterUpdated, Channel);
}

void test_synth_calls_callback_when_lfo_freq_changes(UNUSED void** state)
{
    synth_setParameterUpdateCallback(&updateCallback);

    expect_ym2612_write_reg_any_data(0, YM_LFO_ENABLE);
    __real_synth_globalLfoFrequency(1);

    assert_true(updated);
    assert_int_equal(lastParameterUpdated, Lfo);
}

void test_synth_calls_callback_when_lfo_enable_changes(UNUSED void** state)
{
    synth_setParameterUpdateCallback(&updateCallback);

    expect_ym2612_write_reg_any_data(0, YM_LFO_ENABLE);
    __real_synth_enableLfo(0);

    assert_true(updated);
    assert_int_equal(lastParameterUpdated, Lfo);
}

void test_synth_calls_callback_when_special_mode_changes(UNUSED void** state)
{
    synth_setParameterUpdateCallback(&updateCallback);

    expect_ym2612_write_reg_any_data(0, YM_CH3_MODE);
    __real_synth_setSpecialMode(true);

    assert_true(updated);
    assert_int_equal(lastParameterUpdated, SpecialMode);
}

void test_synth_enables_ch3_special_mode(UNUSED void** state)
{
    expect_ym2612_write_reg(0, YM_CH3_MODE, 0x40);
    __real_synth_setSpecialMode(true);
}

void test_synth_disables_ch3_special_mode(UNUSED void** state)
{
    expect_ym2612_write_reg(0, YM_CH3_MODE, 0);
    __real_synth_setSpecialMode(false);
}

void test_synth_sets_ch3_special_mode_operator_pitches(UNUSED void** state)
{
    const u8 upperRegs[]
        = { YM_CH3SM_OP1_FREQ_MSB_BLK, YM_CH3SM_OP3_FREQ_MSB_BLK, YM_CH3SM_OP2_FREQ_MSB_BLK };
    const u8 lowerRegs[] = { YM_CH3SM_OP1_FREQ_LSB, YM_CH3SM_OP3_FREQ_LSB, YM_CH3SM_OP2_FREQ_LSB };

    for (u8 op = 0; op < 3; op++) {
        expect_ym2612_write_reg(0, upperRegs[op], 0x22);
        expect_ym2612_write_reg(0, lowerRegs[op], 0x84);

        __real_synth_specialModePitch(op, 4, SYNTH_NTSC_C);
    }
}

void test_synth_handles_out_of_range_ch3_special_mode_operator(UNUSED void** state)
{
    const u8 op = 3; // invalid op
    expect_ym2612_write_reg(0, YM_CH3SM_OP1_FREQ_MSB_BLK, 0x22); // safely wrap to valid reg
    expect_ym2612_write_reg(0, YM_CH3SM_OP1_FREQ_LSB, 0x84);

    __real_synth_specialModePitch(op, 4, SYNTH_NTSC_C);
}

void test_synth_sets_ch3_special_mode_op_tl_only_if_output_operator(UNUSED void** state)
{
    int algorithm = (*(int*)(*state));

    expect_ym2612_write_channel(CH3_SPECIAL_MODE, YM_BASE_ALGORITHM_FEEDBACK, algorithm);
    __real_synth_algorithm(CH3_SPECIAL_MODE, algorithm);

    for (u8 op = 0; op <= 2; op++) {
        // set initial TL
        expect_ym2612_write_operator(CH3_SPECIAL_MODE, op, YM_BASE_TOTAL_LEVEL, 2);
        __real_synth_operatorTotalLevel(CH3_SPECIAL_MODE, op, 2);

        switch (algorithm) {
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
                expect_ym2612_write_operator(CH3_SPECIAL_MODE, op, YM_BASE_TOTAL_LEVEL, 14);
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
                expect_ym2612_write_operator(CH3_SPECIAL_MODE, op, YM_BASE_TOTAL_LEVEL, 14);
                __real_synth_specialModeVolume(op, 60);
            }
            break;
        }
        case 7: {
            // set volume
            expect_ym2612_write_operator(CH3_SPECIAL_MODE, op, YM_BASE_TOTAL_LEVEL, 14);
            __real_synth_specialModeVolume(op, 60);
            break;
        }
        }
    }
}

void test_synth_enables_dac(UNUSED void** state)
{
    expect_ym2612_write_reg(0, YM_DAC_ENABLE, 0x80);

    __real_synth_enableDac(true);
}

void test_synth_disables_dac(UNUSED void** state)
{
    expect_ym2612_write_reg(0, YM_DAC_ENABLE, 0);

    __real_synth_enableDac(false);
}

void test_requests_Z80_bus_if_not_already_taken(UNUSED void** state)
{
    expect_value(__wrap_Z80_getAndRequestBus, wait, TRUE);
    will_return(__wrap_Z80_getAndRequestBus, false);

    expect_value(__wrap_YM2612_writeReg, part, 0);
    expect_value(__wrap_YM2612_writeReg, reg, YM_DAC_ENABLE);
    expect_value(__wrap_YM2612_writeReg, data, 0);

    expect_value(__wrap_YM2612_write, port, 0);
    expect_value(__wrap_YM2612_write, data, YM_DAC_DATA);
    expect_function_call(__wrap_Z80_releaseBus);

    __real_synth_directWriteYm2612(0, YM_DAC_ENABLE, 0);
}

void test_does_not_release_Z80_bus_when_taken_prior_to_call(UNUSED void** state)
{
    expect_value(__wrap_Z80_getAndRequestBus, wait, TRUE);
    will_return(__wrap_Z80_getAndRequestBus, true);

    expect_value(__wrap_YM2612_writeReg, part, 0);
    expect_value(__wrap_YM2612_writeReg, reg, YM_DAC_ENABLE);
    expect_value(__wrap_YM2612_writeReg, data, 0);

    __real_synth_directWriteYm2612(0, YM_DAC_ENABLE, 0);
}
