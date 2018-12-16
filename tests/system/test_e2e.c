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

    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x8D);
    expect_ym2612_write_reg(0, 0x28, 0xF0);

    interface_tick();
}

static void test_midi_pitch_bend_sent_to_ym2612(void** state)
{
    const u8 bendStatus = 0xE0;
    const u16 bend = 12000;
    const u8 bendLower = bend & 0x007F;
    const u8 bendUpper = bend >> 8;

    will_return(__wrap_ssf_usb_rd_ready, 1);
    will_return(__wrap_ssf_usb_read, bendStatus);
    will_return(__wrap_ssf_usb_rd_ready, 1);
    will_return(__wrap_ssf_usb_read, bendLower);
    will_return(__wrap_ssf_usb_rd_ready, 1);
    will_return(__wrap_ssf_usb_read, bendUpper);

    expect_ym2612_write_channel_any_data(0, 0xA4);
    expect_ym2612_write_channel_any_data(0, 0xA0);
    interface_tick();
}
