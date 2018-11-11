
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "midi.h"
#include "psg_chip.h"
#include <cmocka.h>

extern void __real_psg_noteOn(u8 channel, u16 freq, u8 attenuation);

static void test_psg_chip_sets_note_on_psg(void** state)
{
    for (u8 chan = 0; chan < 3; chan++) {
        expect_value(__wrap_PSG_setFrequency, channel, chan);
        expect_value(__wrap_PSG_setFrequency, value, 440);
        expect_value(__wrap_PSG_setEnvelope, channel, chan);
        expect_value(__wrap_PSG_setEnvelope, value, 0);

        __real_psg_noteOn(chan, 440, 0);
    }
}
