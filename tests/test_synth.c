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

static void test_synth_init_sets_initial_registers(void** state)
{
    u16 count = 179;
    expect_any_count(__wrap_fm_writeReg, part, count);
    expect_any_count(__wrap_fm_writeReg, reg, count);
    expect_any_count(__wrap_fm_writeReg, data, count);

    __real_synth_init();
}

static void test_synth_sets_note_on_fm_reg_chan_0_to_2(void** state)
{
    for (u8 chan = 0; chan < 3; chan++) {
        expect_value(__wrap_fm_writeReg, part, 0);
        expect_value(__wrap_fm_writeReg, reg, 0x28);
        expect_value(__wrap_fm_writeReg, data, 0xF0 + chan);

        __real_synth_noteOn(chan);
    }
}

static void test_synth_sets_note_on_fm_reg_chan_3_to_5(void** state)
{
    for (u8 chan = 3; chan < 6; chan++) {
        expect_value(__wrap_fm_writeReg, part, 0);
        expect_value(__wrap_fm_writeReg, reg, 0x28);
        expect_value(__wrap_fm_writeReg, data, 0xF0 + 1 + chan);

        __real_synth_noteOn(chan);
    }
}

static void test_synth_sets_note_off_fm_reg_chan_0_to_2(void** state)
{
    for (u8 chan = 0; chan < 3; chan++) {
        expect_value(__wrap_fm_writeReg, part, 0);
        expect_value(__wrap_fm_writeReg, reg, 0x28);
        expect_value(__wrap_fm_writeReg, data, chan);

        __real_synth_noteOff(chan);
    }
}

static void test_synth_sets_note_off_fm_reg_chan_3_to_5(void** state)
{
    for (u8 chan = 3; chan < 6; chan++) {
        expect_value(__wrap_fm_writeReg, part, 0);
        expect_value(__wrap_fm_writeReg, reg, 0x28);
        expect_value(__wrap_fm_writeReg, data, 1 + chan);

        __real_synth_noteOff(chan);
    }
}

static void test_synth_sets_octave_and_freq_reg_chan(void** state)
{
    for (u8 chan = 0; chan < 6; chan++) {
        u8 regOffset = chan % 3;
        u8 regPart = chan < 3 ? 0 : 1;
        expect_value(__wrap_fm_writeReg, part, regPart);
        expect_value(__wrap_fm_writeReg, reg, 0xA4 + regOffset);
        expect_value(__wrap_fm_writeReg, data, 0x22);

        expect_value(__wrap_fm_writeReg, part, regPart);
        expect_value(__wrap_fm_writeReg, reg, 0xA0 + regOffset);
        expect_value(__wrap_fm_writeReg, data, 0x8D);

        __real_synth_pitch(chan, 4, 653);
    }
}

static void test_synth_sets_total_level_reg_chan(void** state)
{
    for (u8 chan = 0; chan < 6; chan++) {
        u8 regOffset = chan % 3;
        u8 regPart = chan < 3 ? 0 : 1;
        expect_value(__wrap_fm_writeReg, part, regPart);
        expect_value(__wrap_fm_writeReg, reg, 0x4C + regOffset);
        expect_value(__wrap_fm_writeReg, data, 0);

        __real_synth_totalLevel(chan, 0);
    }
}
