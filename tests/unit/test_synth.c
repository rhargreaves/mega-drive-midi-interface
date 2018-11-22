#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

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
extern void __real_synth_operatorFirstDecayRate(u8 channel, u8 op, u8 firstDecayRate);

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
        expect_YM2612_writeReg(0, 0x28, 0xF0 + chan);
        __real_synth_noteOn(chan);
    }
}

static void test_synth_sets_note_on_fm_reg_chan_3_to_5(void** state)
{
    for (u8 chan = 3; chan < 6; chan++) {
        expect_YM2612_writeReg(0, 0x28, 0xF1 + chan);
        __real_synth_noteOn(chan);
    }
}

static void test_synth_sets_note_off_fm_reg_chan_0_to_2(void** state)
{
    for (u8 chan = 0; chan < 3; chan++) {
        expect_value(__wrap_YM2612_writeReg, part, 0);
        expect_value(__wrap_YM2612_writeReg, reg, 0x28);
        expect_value(__wrap_YM2612_writeReg, data, chan);

        __real_synth_noteOff(chan);
    }
}

static void test_synth_sets_note_off_fm_reg_chan_3_to_5(void** state)
{
    for (u8 chan = 3; chan < 6; chan++) {
        expect_value(__wrap_YM2612_writeReg, part, 0);
        expect_value(__wrap_YM2612_writeReg, reg, 0x28);
        expect_value(__wrap_YM2612_writeReg, data, 1 + chan);

        __real_synth_noteOff(chan);
    }
}

static void test_synth_sets_octave_and_freq_reg_chan(void** state)
{
    for (u8 chan = 0; chan < 6; chan++) {
        u8 regOffset = chan % 3;
        u8 regPart = chan < 3 ? 0 : 1;
        expect_value(__wrap_YM2612_writeReg, part, regPart);
        expect_value(__wrap_YM2612_writeReg, reg, 0xA4 + regOffset);
        expect_value(__wrap_YM2612_writeReg, data, 0x22);

        expect_value(__wrap_YM2612_writeReg, part, regPart);
        expect_value(__wrap_YM2612_writeReg, reg, 0xA0 + regOffset);
        expect_value(__wrap_YM2612_writeReg, data, 0x8D);

        __real_synth_pitch(chan, 4, 653);
    }
}

static void test_synth_sets_total_level_reg_chan(void** state)
{
    for (u8 chan = 0; chan < 6; chan++) {
        u8 regOffset = chan % 3;
        u8 regPart = chan < 3 ? 0 : 1;
        expect_value(__wrap_YM2612_writeReg, part, regPart);
        expect_value(__wrap_YM2612_writeReg, reg, 0x4C + regOffset);
        expect_value(__wrap_YM2612_writeReg, data, 0);

        __real_synth_totalLevel(chan, 0);
    }
}

static void test_synth_sets_stereo_reg_chan(void** state)
{
    u8 stereo = 1;
    for (u8 chan = 0; chan < 6; chan++) {
        u8 regOffset = chan % 3;
        u8 regPart = chan < 3 ? 0 : 1;
        expect_value(__wrap_YM2612_writeReg, part, regPart);
        expect_value(__wrap_YM2612_writeReg, reg, 0xB4 + regOffset);
        expect_value(__wrap_YM2612_writeReg, data, stereo << 6);

        __real_synth_stereo(chan, stereo);
    }
}

static void test_synth_sets_algorithm(void** state)
{
    u8 defaultFeedback = 6;
    u8 algorithm = 1;
    for (u8 chan = 0; chan < 6; chan++) {
        u8 regOffset = chan % 3;
        u8 regPart = chan < 3 ? 0 : 1;
        expect_value(__wrap_YM2612_writeReg, part, regPart);
        expect_value(__wrap_YM2612_writeReg, reg, 0xB0 + regOffset);
        expect_value(__wrap_YM2612_writeReg, data, (defaultFeedback << 3) + algorithm);

        __real_synth_algorithm(chan, algorithm);
    }
}

static void test_synth_sets_feedback(void** state)
{
    u8 defaultAlgorithm = 2;
    u8 feedback = 1;
    for (u8 chan = 0; chan < 6; chan++) {
        u8 regOffset = chan % 3;
        u8 regPart = chan < 3 ? 0 : 1;
        expect_value(__wrap_YM2612_writeReg, part, regPart);
        expect_value(__wrap_YM2612_writeReg, reg, 0xB0 + regOffset);
        expect_value(__wrap_YM2612_writeReg, data, (feedback << 3) + defaultAlgorithm);

        __real_synth_feedback(chan, feedback);
    }
}

static void test_synth_sets_feedback_and_algorithm(void** state)
{
    u8 defaultFeedback = 6;

    for (u8 chan = 0; chan < 6; chan++) {
        u8 feedback = 1;
        u8 algorithm = 1;
        u8 regOffset = chan % 3;
        u8 regPart = chan < 3 ? 0 : 1;
        expect_value(__wrap_YM2612_writeReg, part, regPart);
        expect_value(__wrap_YM2612_writeReg, reg, 0xB0 + regOffset);
        expect_value(__wrap_YM2612_writeReg, data, (defaultFeedback << 3) + algorithm);

        __real_synth_algorithm(chan, feedback);

        expect_value(__wrap_YM2612_writeReg, part, regPart);
        expect_value(__wrap_YM2612_writeReg, reg, 0xB0 + regOffset);
        expect_value(__wrap_YM2612_writeReg, data, (feedback << 3) + algorithm);

        __real_synth_feedback(chan, feedback);

        feedback++;
        algorithm++;
    }
}

static void test_synth_sets_operator_total_level(void** state)
{
    u8 totalLevel = 50;
    for (u8 chan = 0; chan < 6; chan++) {
        u8 regOffset = chan % 3;
        u8 regPart = chan < 3 ? 0 : 1;
        for (u8 op = 0; op < 4; op++) {
            expect_value(__wrap_YM2612_writeReg, part, regPart);
            expect_value(__wrap_YM2612_writeReg, reg, 0x40 + regOffset + (op * 4));
            expect_value(__wrap_YM2612_writeReg, data, totalLevel);

            __real_synth_operatorTotalLevel(chan, op, totalLevel);
        }
    }
}

static void test_synth_sets_operator_multiple_and_detune(void** state)
{
    for (u8 chan = 0; chan < 6; chan++) {
        u8 multiple = 0;
        u8 detune = 0;
        u8 regOffset = chan % 3;
        u8 regPart = chan < 3 ? 0 : 1;
        for (u8 op = 0; op < 4; op++) {
            expect_value(__wrap_YM2612_writeReg, part, regPart);
            expect_value(__wrap_YM2612_writeReg, reg, 0x30 + regOffset + (op * 4));
            expect_any(__wrap_YM2612_writeReg, data);

            __real_synth_operatorMultiple(chan, op, multiple);

            expect_value(__wrap_YM2612_writeReg, part, regPart);
            expect_value(__wrap_YM2612_writeReg, reg, 0x30 + regOffset + (op * 4));
            expect_value(__wrap_YM2612_writeReg, data, (detune << 4) | multiple);

            __real_synth_operatorDetune(chan, op, detune);

            multiple++;
            detune++;
        }
    }
}

static void test_synth_sets_operator_attack_rate_and_rate_scaling(void** state)
{
    for (u8 chan = 0; chan < 6; chan++) {
        u8 attackRate = 0;
        u8 rateScaling = 0;
        u8 regOffset = chan % 3;
        u8 regPart = chan < 3 ? 0 : 1;
        for (u8 op = 0; op < 4; op++) {
            expect_value(__wrap_YM2612_writeReg, part, regPart);
            expect_value(__wrap_YM2612_writeReg, reg, 0x50 + regOffset + (op * 4));
            expect_any(__wrap_YM2612_writeReg, data);

            __real_synth_operatorAttackRate(chan, op, attackRate);

            expect_value(__wrap_YM2612_writeReg, part, regPart);
            expect_value(__wrap_YM2612_writeReg, reg, 0x50 + regOffset + (op * 4));
            expect_value(__wrap_YM2612_writeReg, data, attackRate | (rateScaling << 6));

            __real_synth_operatorRateScaling(chan, op, rateScaling);

            attackRate++;
            rateScaling++;
        }
    }
}

static void test_synth_sets_operator_first_decay_rate(void** state)
{
    u8 firstDecayRate = 16;
    for (u8 chan = 0; chan < 6; chan++) {
        u8 regOffset = chan % 3;
        u8 regPart = chan < 3 ? 0 : 1;
        for (u8 op = 0; op < 4; op++) {
            expect_value(__wrap_YM2612_writeReg, part, regPart);
            expect_value(__wrap_YM2612_writeReg, reg, 0x60 + regOffset + (op * 4));
            expect_value(__wrap_YM2612_writeReg, data, firstDecayRate);

            __real_synth_operatorFirstDecayRate(chan, op, firstDecayRate);
        }
    }
}
