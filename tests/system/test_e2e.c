#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "asserts.h"
#include "comm.h"
#include "midi.h"
#include "midi_receiver.h"
#include "presets.h"
#include "wraps.h"
#include <cmocka.h>

static const u8 SYSEX_START = 0xF0;
static const u8 SYSEX_EXTENDED_MANU_ID_SECTION = 0x00;
static const u8 SYSEX_UNUSED_EUROPEAN_SECTION = 0x22;
static const u8 SYSEX_UNUSED_MANU_ID = 0x77;
static const u8 SYSEX_END = 0xF7;

static int test_e2e_setup(void** state)
{
    comm_resetCounts();
    comm_init();
    const u16 times = 187;
    expect_any_count(__wrap_YM2612_writeReg, part, times);
    expect_any_count(__wrap_YM2612_writeReg, reg, times);
    expect_any_count(__wrap_YM2612_writeReg, data, times);
    midi_init((Channel**)M_BANK_0, (PercussionPreset**)P_BANK_0);
    return 0;
}

static void expect_total_level_set_for_current_algorithm(u8 channel)
{
    expect_ym2612_write_operator_any_data(channel, 0, 0x40);
    expect_ym2612_write_operator_any_data(channel, 0, 0x44);
    expect_ym2612_write_operator_any_data(channel, 0, 0x48);
    expect_ym2612_write_operator_any_data(channel, 0, 0x4C);
}

static void test_midi_note_on_event_sent_to_ym2612(void** state)
{
    const u8 noteOnStatus = 0x90;
    const u8 noteOnKey = 48;
    const u8 noteOnVelocity = 127;

    stub_usb_receive_byte(noteOnStatus);
    stub_usb_receive_byte(noteOnKey);
    stub_usb_receive_byte(noteOnVelocity);

    expect_total_level_set_for_current_algorithm(0);
    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x8D);

    expect_ym2612_write_reg(0, 0x28, 0xF0);

    midi_receiver_read();
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

    midi_receiver_read();
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

    midi_receiver_read();

    stub_usb_receive_byte(noteOnStatus);
    stub_usb_receive_byte(noteOnKey1);
    stub_usb_receive_byte(noteOnVelocity);

    expect_total_level_set_for_current_algorithm(0);
    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x8D);
    expect_ym2612_write_reg(0, 0x28, 0xF0);

    midi_receiver_read();

    stub_usb_receive_byte(noteOnStatus);
    stub_usb_receive_byte(noteOnKey2);
    stub_usb_receive_byte(noteOnVelocity);

    expect_total_level_set_for_current_algorithm(1);
    expect_ym2612_write_channel(1, 0xA4, 0x1A);
    expect_ym2612_write_channel(1, 0xA0, 0xB4);
    expect_ym2612_write_reg(0, 0x28, 0xF1);

    midi_receiver_read();
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

    midi_receiver_read();
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

    midi_receiver_read();
}

static void test_general_midi_reset_sysex_stops_all_notes(void** state)
{
    print_message("Playing note\n");
    const u8 noteOnStatus = 0x90;
    const u8 noteOnKey = 48;
    const u8 noteOnVelocity = 127;

    stub_usb_receive_byte(noteOnStatus);
    stub_usb_receive_byte(noteOnKey);
    stub_usb_receive_byte(noteOnVelocity);

    expect_total_level_set_for_current_algorithm(0);
    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x8D);
    expect_ym2612_write_reg(0, 0x28, 0xF0);

    midi_receiver_read();

    stub_usb_receive_byte(noteOnStatus + MIN_PSG_CHAN);
    stub_usb_receive_byte(noteOnKey);
    stub_usb_receive_byte(noteOnVelocity);

    expect_value(__wrap_PSG_setFrequency, channel, 0);
    expect_any(__wrap_PSG_setFrequency, value);
    expect_value(__wrap_PSG_setEnvelope, channel, 0);
    expect_value(__wrap_PSG_setEnvelope, value, 0);

    midi_receiver_read();

    print_message("Sending reset\n");
    const u8 sysExGeneralMidiResetSequence[]
        = { 0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7 };
    for (int i = 0; i < sizeof(sysExGeneralMidiResetSequence); i++) {
        stub_usb_receive_byte(sysExGeneralMidiResetSequence[i]);
    }

    expect_ym2612_write_reg(0, 0x28, 0x00);
    expect_ym2612_write_reg(0, 0x28, 0x01);
    expect_ym2612_write_reg(0, 0x28, 0x02);
    expect_ym2612_write_reg(0, 0x28, 0x04);
    expect_ym2612_write_reg(0, 0x28, 0x05);
    expect_ym2612_write_reg(0, 0x28, 0x06);
    expect_value(__wrap_PSG_setEnvelope, channel, 0);
    expect_value(__wrap_PSG_setEnvelope, value, 0xF);
    midi_receiver_read();
}

static void test_remap_midi_channel_1_to_psg_channel_1()
{
    const u8 SYSEX_REMAP_COMMAND_ID = 0x00;
    const u8 SYSEX_REMAP_MIDI_CHANNEL = 0x00;
    const u8 SYSEX_REMAP_DESTINATION_FIRST_PSG_CHANNEL = 0x06;

    const u8 sysExRemapSequence[] = { SYSEX_START,
        SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
        SYSEX_UNUSED_MANU_ID, SYSEX_REMAP_COMMAND_ID, SYSEX_REMAP_MIDI_CHANNEL,
        SYSEX_REMAP_DESTINATION_FIRST_PSG_CHANNEL, SYSEX_END };
    for (int i = 0; i < sizeof(sysExRemapSequence); i++) {
        stub_usb_receive_byte(sysExRemapSequence[i]);
    }

    midi_receiver_read();

    const u8 noteOnStatus = 0x90;
    const u8 noteOnKey = 48;
    const u8 noteOnVelocity = 127;

    stub_usb_receive_byte(noteOnStatus);
    stub_usb_receive_byte(noteOnKey);
    stub_usb_receive_byte(noteOnVelocity);

    expect_value(__wrap_PSG_setFrequency, channel, 0);
    expect_any(__wrap_PSG_setFrequency, value);
    expect_value(__wrap_PSG_setEnvelope, channel, 0);
    expect_value(__wrap_PSG_setEnvelope, value, 0);

    midi_receiver_read();
}

static void test_pong_received_after_ping_sent()
{
    const u8 SYSEX_PING_COMMAND_ID = 0x01;
    const u8 SYSEX_PONG_COMMAND_ID = 0x02;

    const u8 sysExPingSequence[] = { SYSEX_START,
        SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
        SYSEX_UNUSED_MANU_ID, SYSEX_PING_COMMAND_ID, SYSEX_END };

    const u8 sysExPongSequence[] = { SYSEX_START,
        SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
        SYSEX_UNUSED_MANU_ID, SYSEX_PONG_COMMAND_ID, SYSEX_END };

    for (int i = 0; i < sizeof(sysExPingSequence); i++) {
        stub_usb_receive_byte(sysExPingSequence[i]);
    }

    for (int i = 0; i < sizeof(sysExPongSequence); i++) {
        expect_usb_sent_byte(sysExPongSequence[i]);
    }

    midi_receiver_read();
}
