
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "midi.h"
#include "psg_chip.h"
#include <cmocka.h>

extern void __real_psg_attenuation(u8 channel, u8 attenuation);
extern void __real_psg_noteOn(u8 channel, u16 freq);
extern void __real_psg_noteOff(u8 channel);
extern void __real_psg_frequency(u8 channel, u16 freq);

static void test_psg_chip_sets_attenuation(void** state)
{
    for (u8 chan = 0; chan < MAX_PSG_CHANS; chan++) {
        expect_value(__wrap_PSG_setEnvelope, channel, chan);
        expect_value(__wrap_PSG_setEnvelope, value, 2);

        __real_psg_attenuation(chan, 2);
    }
}

static void test_psg_chip_sets_frequency(void** state)
{
    for (u8 chan = 0; chan < 3; chan++) {
        expect_value(__wrap_PSG_setFrequency, channel, chan);
        expect_value(__wrap_PSG_setFrequency, value, 440);

        __real_psg_frequency(chan, 440);
    }
}

static void test_psg_sets_busy_indicators(void** state)
{
    for (u8 chan = 0; chan < MAX_PSG_CHANS; chan++) {
        expect_any(__wrap_PSG_setEnvelope, channel);
        expect_any(__wrap_PSG_setEnvelope, value);
        __real_psg_attenuation(chan, 0xf);
    }

    for (u8 chan = 0; chan < MAX_PSG_CHANS; chan += 2) {
        expect_value(__wrap_PSG_setEnvelope, channel, chan);
        expect_value(__wrap_PSG_setEnvelope, value, 2);
        __real_psg_attenuation(chan, 2);
    }
    u8 busy = psg_busy();
    assert_int_equal(busy, 0b0101);
}
