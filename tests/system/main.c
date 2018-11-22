#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "interface.h"
#include <cmocka.h>

static void test_midi_note_on_event_sent_to_ym2612(void** state)
{
    const u8 noteOnStatus = 0x90;
    const u8 noteOnKey = 60;
    const u8 noteOnVelocity = 127;

    will_return(__wrap_ssf_usb_rd_ready, 1);
    will_return(__wrap_ssf_usb_read, noteOnStatus);
    will_return(__wrap_ssf_usb_rd_ready, 1);
    will_return(__wrap_ssf_usb_read, noteOnKey);
    will_return(__wrap_ssf_usb_rd_ready, 1);
    will_return(__wrap_ssf_usb_read, noteOnVelocity);

    const u8 regPart = 0;
    const u8 regOffset = 0;

    expect_value(__wrap_YM2612_writeReg, part, regPart);
    expect_value(__wrap_YM2612_writeReg, reg, 0xA4 + regOffset);
    expect_value(__wrap_YM2612_writeReg, data, 0x1A);

    expect_value(__wrap_YM2612_writeReg, part, regPart);
    expect_value(__wrap_YM2612_writeReg, reg, 0xA0 + regOffset);
    expect_value(__wrap_YM2612_writeReg, data, 0x8D);

    expect_value(__wrap_YM2612_writeReg, part, regPart);
    expect_value(__wrap_YM2612_writeReg, reg, 0x28);
    expect_value(__wrap_YM2612_writeReg, data, 0xF0 + regOffset);

    interface_tick();
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_midi_note_on_event_sent_to_ym2612)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
