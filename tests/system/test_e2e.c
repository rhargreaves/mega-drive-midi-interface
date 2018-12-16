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

    stub_usb_receive_byte(noteOnStatus);
    stub_usb_receive_byte(noteOnKey);
    stub_usb_receive_byte(noteOnVelocity);

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

    stub_usb_receive_byte(bendStatus);
    stub_usb_receive_byte(bendLower);
    stub_usb_receive_byte(bendUpper);

    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x77);

    interface_tick();
}

static void test_polyphonic_midi_sent_to_separate_ym2612_channels(void** state)
{
    const u8 noteOnStatus = 0x90;
    const u8 ccStatus = 0xB0;
    const u8 ccPolyphonic = 80;
    const u8 ccPolyphonicOnValue = 0x7F;
    const u8 noteOnKey1 = 60;
    const u8 noteOnKey2 = 60;
    const u8 noteOnVelocity = 127;

    stub_usb_receive_byte(ccStatus);
    stub_usb_receive_byte(ccPolyphonic);
    stub_usb_receive_byte(ccPolyphonicOnValue);

    interface_tick();

    stub_usb_receive_byte(noteOnStatus);
    stub_usb_receive_byte(noteOnKey1);
    stub_usb_receive_byte(noteOnVelocity);

    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x8D);
    expect_ym2612_write_reg(0, 0x28, 0xF0);

    interface_tick();

    stub_usb_receive_byte(noteOnStatus);
    stub_usb_receive_byte(noteOnKey2);
    stub_usb_receive_byte(noteOnVelocity);

    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x8D);
    expect_ym2612_write_reg(0, 0x28, 0xF1);

    interface_tick();
}
