#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "asserts.h"
#include "comm.h"
#include "envelopes.h"
#include "midi.h"
#include "midi_receiver.h"
#include "presets.h"
#include "wraps.h"
#include <cmocka.h>

static int test_e2e_setup(void** state)
{
    wraps_disable_checks();
    comm_reset_counts();
    comm_init();
    midi_init(M_BANK_0, P_BANK_0, ENVELOPES);
    wraps_enable_checks();
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

    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x8D);
    expect_ym2612_write_reg(0, 0x28, 0xF0);

    midi_receiver_read();

    stub_usb_receive_byte(noteOnStatus);
    stub_usb_receive_byte(noteOnKey2);
    stub_usb_receive_byte(noteOnVelocity);

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

    expect_any(__wrap_PSG_setTone, channel);
    expect_any(__wrap_PSG_setTone, value);
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

    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x8D);
    expect_ym2612_write_reg(0, 0x28, 0xF0);

    midi_receiver_read();

    stub_usb_receive_byte(noteOnStatus + MIN_PSG_CHAN);
    stub_usb_receive_byte(noteOnKey);
    stub_usb_receive_byte(noteOnVelocity);

    expect_value(__wrap_PSG_setTone, channel, 0);
    expect_any(__wrap_PSG_setTone, value);
    expect_value(__wrap_PSG_setEnvelope, channel, 0);
    expect_value(__wrap_PSG_setEnvelope, value, 0);

    midi_receiver_read();

    print_message("Sending reset\n");
    const u8 sysExGeneralMidiResetSequence[]
        = { 0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7 };
    for (u16 i = 0; i < sizeof(sysExGeneralMidiResetSequence); i++) {
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

static void remapChannel(u8 midiChannel, u8 deviceChannel)
{
    u8 sysExRemapSequence[]
        = { SYSEX_START, SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
              SYSEX_COMMAND_REMAP, midiChannel, deviceChannel, SYSEX_END };
    for (u16 i = 0; i < sizeof(sysExRemapSequence); i++) {
        stub_usb_receive_byte(sysExRemapSequence[i]);
    }
}

static void test_remap_midi_channel_1_to_psg_channel_1()
{
    const u8 MIDI_CHANNEL_1 = 0;
    const u8 MIDI_CHANNEL_UNASSIGNED = 0x7F;
    const u8 PSG_TONE_1 = 0x06;
    const u8 FM_CHAN_1 = 0;

    remapChannel(MIDI_CHANNEL_UNASSIGNED, FM_CHAN_1);
    midi_receiver_read();
    remapChannel(MIDI_CHANNEL_1, PSG_TONE_1);
    midi_receiver_read();

    const u8 noteOnStatus = 0x90;
    const u8 noteOnKey = 48;
    const u8 noteOnVelocity = 127;

    stub_usb_receive_byte(noteOnStatus);
    stub_usb_receive_byte(noteOnKey);
    stub_usb_receive_byte(noteOnVelocity);

    expect_value(__wrap_PSG_setTone, channel, 0);
    expect_any(__wrap_PSG_setTone, value);
    expect_value(__wrap_PSG_setEnvelope, channel, 0);
    expect_value(__wrap_PSG_setEnvelope, value, 0);

    midi_receiver_read();
}

static void test_set_device_for_midi_channel_1_to_psg()
{
    const u8 noteOnStatus = 0x90;
    const u8 ccStatus = 0xB0;
    const u8 ccPolyphonic = 80;
    const u8 ccPolyphonicOnValue = 0x7F;
    const u8 ccDeviceSelect = 86;
    const u8 ccDevicePsgValue = 64;
    const u8 noteOnKey = 48;
    const u8 noteOnVelocity = 127;

    stub_usb_receive_byte(ccStatus);
    stub_usb_receive_byte(ccPolyphonic);
    stub_usb_receive_byte(ccPolyphonicOnValue);

    midi_receiver_read();

    stub_usb_receive_byte(ccStatus);
    stub_usb_receive_byte(ccDeviceSelect);
    stub_usb_receive_byte(ccDevicePsgValue);

    midi_receiver_read();

    stub_usb_receive_byte(noteOnStatus);
    stub_usb_receive_byte(noteOnKey);
    stub_usb_receive_byte(noteOnVelocity);

    expect_value(__wrap_PSG_setTone, channel, 0);
    expect_any(__wrap_PSG_setTone, value);
    expect_value(__wrap_PSG_setEnvelope, channel, 0);
    expect_value(__wrap_PSG_setEnvelope, value, 0);

    midi_receiver_read();
}

static void test_pong_received_after_ping_sent()
{
    const u8 sysExPingSequence[] = { SYSEX_START, SYSEX_MANU_EXTENDED,
        SYSEX_MANU_REGION, SYSEX_MANU_ID, SYSEX_COMMAND_PING, SYSEX_END };

    const u8 sysExPongSequence[] = { SYSEX_START, SYSEX_MANU_EXTENDED,
        SYSEX_MANU_REGION, SYSEX_MANU_ID, SYSEX_COMMAND_PONG, SYSEX_END };

    for (u16 i = 0; i < sizeof(sysExPingSequence); i++) {
        stub_usb_receive_byte(sysExPingSequence[i]);
    }

    for (u16 i = 0; i < sizeof(sysExPongSequence); i++) {
        expect_usb_sent_byte(sysExPongSequence[i]);
    }

    midi_receiver_read();
}

static void test_loads_psg_envelope()
{
    const u8 sysExPingSequence[]
        = { SYSEX_START, SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
              SYSEX_COMMAND_LOAD_PSG_ENVELOPE, 0x06, 0x06, SYSEX_END };

    for (u16 i = 0; i < sizeof(sysExPingSequence); i++) {
        stub_usb_receive_byte(sysExPingSequence[i]);
    }

    midi_receiver_read();

    const u8 psgMidiChannel1 = 6;
    const u8 noteOnStatus = 0x90 + psgMidiChannel1;
    const u8 noteOnKey = 60;
    const u8 noteOnVelocity = 127;

    stub_usb_receive_byte(noteOnStatus);
    stub_usb_receive_byte(noteOnKey);
    stub_usb_receive_byte(noteOnVelocity);

    expect_value(__wrap_PSG_setTone, channel, 0);
    expect_value(__wrap_PSG_setTone, value, 0x17c);
    expect_value(__wrap_PSG_setEnvelope, channel, 0);
    expect_value(__wrap_PSG_setEnvelope, value, 6);

    midi_receiver_read();
}
