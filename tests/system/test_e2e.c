#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "asserts.h"
#include "comm.h"
#include "interface.h"
#include "midi.h"
#include "wraps.h"
#include <cmocka.h>

static int test_e2e_setup(void** state)
{
    comm_resetCounts();
    midi_reset();
    midi_psg_reset();
    return 0;
}

static void test_midi_note_on_event_sent_to_ym2612(void** state)
{
    const u8 noteOnStatus = 0x90;
    const u8 noteOnKey = 48;
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
    const u8 noteOnKey1 = 48;
    const u8 noteOnKey2 = 49;
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

    expect_ym2612_write_channel(0, 0xA5, 0x1A);
    expect_ym2612_write_channel(0, 0xA1, 0xB4);
    expect_ym2612_write_reg(0, 0x28, 0xF1);

    interface_tick();
}

static void test_psg_audible_if_note_on_event_triggered(void** state)
{
    const u8 psgMidiChannel1 = 6;
    const u8 noteOnStatus = 0x90 + psgMidiChannel1;
    const u8 noteOnKey = 60;
    const u8 noteOnVelocity = 127;

    stub_usb_receive_byte(noteOnStatus);
    stub_usb_receive_byte(noteOnKey);
    stub_usb_receive_byte(noteOnVelocity);

    expect_any(__wrap_PSG_setFrequency, channel);
    expect_any(__wrap_PSG_setFrequency, value);
    expect_any(__wrap_PSG_setEnvelope, channel);
    expect_any(__wrap_PSG_setEnvelope, value);

    interface_tick();
}

static void
test_psg_not_audible_if_midi_channel_volume_set_and_there_is_no_note_on_event(
    void** state)
{
    const u8 psgMidiChannel1 = 6;
    const u8 ccStatus = 0xB0 + psgMidiChannel1;
    const u8 ccVolume = 0x7;
    const u8 ccVolumeValue = 127;

    stub_usb_receive_byte(ccStatus);
    stub_usb_receive_byte(ccVolume);
    stub_usb_receive_byte(ccVolumeValue);

    interface_tick();
}
