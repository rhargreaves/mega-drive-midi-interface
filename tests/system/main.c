#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "asserts.h"
#include "interface.h"
#include "wraps.h"
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

    expect_ym2612_writeChannel(0, 0xA4, 0x1A);
    expect_ym2612_writeChannel(0, 0xA0, 0x8D);
    expect_YM2612_writeReg(0, 0x28, 0xF0);

    interface_tick();
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_midi_note_on_event_sent_to_ym2612)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
