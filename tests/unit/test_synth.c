#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "asserts.h"
#include "synth.h"
#include <cmocka.h>
#include <types.h>

extern void __real_synth_init(void);
extern void __real_synth_noteOn(u8 channel);
extern void __real_synth_noteOff(u8 channel);
extern void __real_synth_pitch(u8 channel, u8 octave, u16 freqNumber);
extern void __real_synth_totalLevel(u8 channel, u8 totalLevel);
extern void __real_synth_stereo(u8 channel, u8 stereo);
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

static int test_synth_setup(void** state)
{
    const u16 count = 167;
    expect_any_count(__wrap_YM2612_writeReg, part, count);
    expect_any_count(__wrap_YM2612_writeReg, reg, count);
    expect_any_count(__wrap_YM2612_writeReg, data, count);

    __real_synth_init();
}

static void test_synth_init_sets_initial_registers(void** state)
{
    const u16 count = 167;
    expect_any_count(__wrap_YM2612_writeReg, part, count);
    expect_any_count(__wrap_YM2612_writeReg, reg, count);
    expect_any_count(__wrap_YM2612_writeReg, data, count);

    __real_synth_init();
}

static void test_synth_sets_note_on_fm_reg_chan_0_to_2(void** state)
{
    for (u8 chan = 0; chan < 3; chan++) {
        expect_ym2612_write_reg(0, 0x28, 0xF0 + chan);
        __real_synth_noteOn(chan);
    }
}

static void test_synth_sets_note_on_fm_reg_chan_3_to_5(void** state)
{
    for (u8 chan = 3; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_reg(0, 0x28, 0xF1 + chan);
        __real_synth_noteOn(chan);
    }
}

static void test_synth_sets_note_off_fm_reg_chan_0_to_2(void** state)
{
    for (u8 chan = 0; chan < 3; chan++) {
        expect_ym2612_write_reg(0, 0x28, chan);
        __real_synth_noteOff(chan);
    }
}

static void test_synth_sets_note_off_fm_reg_chan_3_to_5(void** state)
{
    for (u8 chan = 3; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_reg(0, 0x28, 1 + chan);
        __real_synth_noteOff(chan);
    }
}

static void test_synth_sets_octave_and_freq_reg_chan(void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(chan, 0xA4, 0x22);
        expect_ym2612_write_channel(chan, 0xA0, 0x8D);
        __real_synth_pitch(chan, 4, 653);
    }
}

static void test_synth_sets_total_level_reg_chan(void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(chan, 0x4C, 0);
        __real_synth_totalLevel(chan, 0);
    }
}

static void test_synth_sets_stereo_reg_chan(void** state)
{
    const u8 stereo = 1;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(chan, 0xB4, stereo << 6);
        __real_synth_stereo(chan, stereo);
    }
}

static void test_synth_sets_algorithm(void** state)
{
    const u8 defaultFeedback = 6;
    const u8 algorithm = 1;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(
            chan, 0xB0, (defaultFeedback << 3) + algorithm);
        __real_synth_algorithm(chan, algorithm);
    }
}

static void test_synth_sets_feedback(void** state)
{
    const u8 defaultAlgorithm = 2;
    const u8 feedback = 1;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_write_channel(
            chan, 0xB0, (feedback << 3) + defaultAlgorithm);
        __real_synth_feedback(chan, feedback);
    }
}

static void test_synth_sets_feedback_and_algorithm(void** state)
{
    const u8 baseReg = 0xB0;
    const u8 defaultFeedback = 6;
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

static void test_synth_sets_operator_total_level(void** state)
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

static void test_synth_sets_operator_multiple_and_detune(void** state)
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

static void test_synth_sets_operator_attack_rate_and_rate_scaling(void** state)
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

static void test_synth_sets_operator_first_decay_rate(void** state)
{
    const u8 baseReg = 0x60;
    u8 firstDecayRate = 16;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(chan, op, baseReg, firstDecayRate);
            __real_synth_operatorFirstDecayRate(chan, op, firstDecayRate);
        }
    }
}

static void test_synth_sets_operator_second_decay_rate(void** state)
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

static void test_synth_sets_operator_secondary_amplitude(void** state)
{
    const u8 baseReg = 0x80;
    u8 secondaryAmplitude = 16;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            expect_ym2612_write_operator(
                chan, op, baseReg, secondaryAmplitude << 4);
            __real_synth_operatorSecondaryAmplitude(
                chan, op, secondaryAmplitude);
        }
    }
}
