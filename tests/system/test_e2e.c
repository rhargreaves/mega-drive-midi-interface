#include "test_e2e.h"
#include "asserts.h"
#include "comm/comm.h"
#include "envelopes.h"
#include "midi.h"
#include "midi_receiver.h"
#include "presets.h"
#include "wraps.h"
#include "scheduler.h"

static const u8 TEST_CC_PAN = 10;
static const u8 TEST_CC_PORTAMENTO_TIME = 5;
static const u8 TEST_CC_VOLUME = 7;
static const u8 TEST_CC_ALGORITHM = 14;
static const u8 TEST_CC_PORTAMENTO_ON = 65;
static const u8 TEST_CC_ENABLE_DAC = 78;
static const u8 TEST_CC_SPECIAL_MODE = 80;
static const u8 TEST_CC_POLYPHONIC = 84;
static const u8 TEST_CC_DEVICE_SELECT = 86;

static const u8 TEST_POLYPHONIC_ON = 0x7F;
static const u8 TEST_SPECIAL_MODE_ON = 64;
static const u8 TEST_DEVICE_SELECT_PSG = 64;
static const u8 TEST_MIDI_CHANNEL_PSG_1 = 6;
static const u8 TEST_MIDI_CHANNEL_1 = 0;
static const u8 TEST_MIDI_CHANNEL_3 = 2;
static const u8 TEST_MIDI_CHANNEL_6 = 5;
static const u8 TEST_MIDI_CHANNEL_11 = 10;

static const u8 TEST_VOLUME_MAX = 127;
static const u8 TEST_VELOCITY_MAX = 127;

int test_e2e_setup(void** state)
{
    wraps_disable_checks();
    scheduler_init();
    comm_reset_counts();
    stub_megawifi_as_not_present();
    comm_init();
    midi_init(M_BANK_0, P_BANK_0, ENVELOPES);
    wraps_enable_checks();
    return 0;
}

void test_midi_note_on_event_sent_to_ym2612(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_1, 48, 127);
    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x84);
    expect_ym2612_write_reg(0, 0x28, 0xF0);
    midi_receiver_read();
}

void test_polyphonic_midi_sent_to_separate_ym2612_channels(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_cc(TEST_MIDI_CHANNEL_1, TEST_CC_POLYPHONIC, TEST_POLYPHONIC_ON);
    midi_receiver_read();

    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_1, 48, TEST_VELOCITY_MAX);
    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x84);
    expect_ym2612_write_reg(0, 0x28, 0xF0);
    midi_receiver_read();

    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_1, 49, TEST_VELOCITY_MAX);
    expect_ym2612_write_channel(1, 0xA4, 0x1A);
    expect_ym2612_write_channel(1, 0xA0, 0xA9);
    expect_ym2612_write_reg(0, 0x28, 0xF1);

    midi_receiver_read();
}

void test_psg_audible_if_note_on_event_triggered(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_PSG_1, 60, TEST_VELOCITY_MAX);
    expect_any(__wrap_PSG_setTone, channel);
    expect_any(__wrap_PSG_setTone, value);
    expect_any(__wrap_PSG_setEnvelope, channel);
    expect_any(__wrap_PSG_setEnvelope, value);
    midi_receiver_read();
}

void test_psg_not_audible_if_midi_channel_volume_set_and_there_is_no_note_on_event(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_cc(TEST_MIDI_CHANNEL_PSG_1, TEST_CC_VOLUME, TEST_VOLUME_MAX);
    midi_receiver_read();
}

void test_general_midi_reset_sysex_stops_all_notes(void** state)
{
    stub_everdrive_as_present();

    const u8 noteOnKey = 48;

    // FM note
    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_1, noteOnKey, TEST_VELOCITY_MAX);
    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x84);
    expect_ym2612_write_reg(0, 0x28, 0xF0);
    midi_receiver_read();

    // PSG note
    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_PSG_1, noteOnKey, TEST_VELOCITY_MAX);
    expect_value(__wrap_PSG_setTone, channel, 0);
    expect_any(__wrap_PSG_setTone, value);
    expect_value(__wrap_PSG_setEnvelope, channel, 0);
    expect_value(__wrap_PSG_setEnvelope, value, 0);
    midi_receiver_read();

    // Send General MIDI reset
    const u8 sysExGeneralMidiResetSequence[] = { 0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7 };
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
    u8 sysExRemapSequence[] = { SYSEX_START, SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_REMAP, midiChannel, deviceChannel, SYSEX_END };
    for (u16 i = 0; i < sizeof(sysExRemapSequence); i++) {
        stub_usb_receive_byte(sysExRemapSequence[i]);
    }
}

void test_remap_midi_channel_1_to_psg_channel_1()
{
    stub_everdrive_as_present();

    const u8 MIDI_CHANNEL_UNASSIGNED = 0x7F;
    const u8 DEVICE_PSG_1 = 0x06;
    const u8 DEVICE_FM_1 = 0;

    remapChannel(MIDI_CHANNEL_UNASSIGNED, DEVICE_FM_1);
    midi_receiver_read();

    remapChannel(TEST_MIDI_CHANNEL_1, DEVICE_PSG_1);
    midi_receiver_read();

    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_1, 48, TEST_VELOCITY_MAX);
    expect_value(__wrap_PSG_setTone, channel, 0);
    expect_any(__wrap_PSG_setTone, value);
    expect_value(__wrap_PSG_setEnvelope, channel, 0);
    expect_value(__wrap_PSG_setEnvelope, value, 0);
    midi_receiver_read();
}

void test_set_device_for_midi_channel_1_to_psg()
{
    stub_everdrive_as_present();
    stub_usb_receive_cc(TEST_MIDI_CHANNEL_1, TEST_CC_POLYPHONIC, TEST_POLYPHONIC_ON);
    midi_receiver_read();

    stub_usb_receive_cc(TEST_MIDI_CHANNEL_1, TEST_CC_DEVICE_SELECT, TEST_DEVICE_SELECT_PSG);
    midi_receiver_read();

    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_1, 48, TEST_VELOCITY_MAX);
    expect_value(__wrap_PSG_setTone, channel, 0);
    expect_any(__wrap_PSG_setTone, value);
    expect_value(__wrap_PSG_setEnvelope, channel, 0);
    expect_value(__wrap_PSG_setEnvelope, value, 0);
    midi_receiver_read();
}

void test_pong_received_after_ping_sent()
{
    const u8 sysExPingSequence[] = { SYSEX_START, SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION,
        SYSEX_MANU_ID, SYSEX_COMMAND_PING, SYSEX_END };

    const u8 sysExPongSequence[] = { SYSEX_START, SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION,
        SYSEX_MANU_ID, SYSEX_COMMAND_PONG, SYSEX_END };

    stub_everdrive_as_present();

    for (u16 i = 0; i < sizeof(sysExPingSequence); i++) {
        stub_usb_receive_byte(sysExPingSequence[i]);
    }

    for (u16 i = 0; i < sizeof(sysExPongSequence); i++) {
        expect_usb_sent_byte(sysExPongSequence[i]);
    }

    midi_receiver_read();
}

void test_loads_psg_envelope()
{
    const u8 sysExPingSequence[] = { SYSEX_START, SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION,
        SYSEX_MANU_ID, SYSEX_COMMAND_LOAD_PSG_ENVELOPE, 0x06, 0x06, SYSEX_END };

    stub_everdrive_as_present();
    for (u16 i = 0; i < sizeof(sysExPingSequence); i++) {
        stub_usb_receive_byte(sysExPingSequence[i]);
    }
    midi_receiver_read();

    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_PSG_1, 60, TEST_VELOCITY_MAX);
    expect_value(__wrap_PSG_setTone, channel, 0);
    expect_value(__wrap_PSG_setTone, value, 0x17c);
    expect_value(__wrap_PSG_setEnvelope, channel, 0);
    expect_value(__wrap_PSG_setEnvelope, value, 6);
    midi_receiver_read();
}

void test_enables_ch3_special_mode(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_cc(TEST_MIDI_CHANNEL_1, TEST_CC_SPECIAL_MODE, TEST_SPECIAL_MODE_ON);
    expect_ym2612_write_reg(0, 0x27, 0x40);
    midi_receiver_read();
}

void test_sets_separate_ch3_operator_frequencies(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_cc(TEST_MIDI_CHANNEL_1, TEST_CC_SPECIAL_MODE, TEST_SPECIAL_MODE_ON);
    expect_ym2612_write_reg(0, 0x27, 0x40);
    midi_receiver_read();

    stub_usb_receive_cc(TEST_MIDI_CHANNEL_3, TEST_CC_ALGORITHM, 0x7F); // alg 7
    expect_ym2612_write_reg(0, 0xB2, 0x7);
    midi_receiver_read();

    const u8 upperRegs[] = { 0xAD, 0xAE, 0xAC }; // ops 0, 1, 2
    const u8 lowerRegs[] = { 0xA9, 0xAA, 0xA8 };
    const u8 tlValues[] = { 0x27, 0x04, 0x24 };

    for (u8 op = 0; op < 3; op++) {
        stub_usb_receive_note_on(TEST_MIDI_CHANNEL_11 + op, 60, 120);
        expect_ym2612_write_reg(0, upperRegs[op], 0x22);
        expect_ym2612_write_reg(0, lowerRegs[op], 0x84);
        expect_ym2612_write_operator(CH_SPECIAL_MODE, op, 0x40, tlValues[op]);
        midi_receiver_read();
    }
}

void test_pitch_bends_ch3_special_mode_operators(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_cc(TEST_MIDI_CHANNEL_1, TEST_CC_SPECIAL_MODE, TEST_SPECIAL_MODE_ON);
    expect_ym2612_write_reg(0, 0x27, 0x40);
    midi_receiver_read();

    stub_usb_receive_cc(TEST_MIDI_CHANNEL_3, TEST_CC_ALGORITHM, 0x7F); // alg 7
    expect_ym2612_write_reg(0, 0xB2, 0x7);
    midi_receiver_read();

    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_11, 60, 120);
    expect_ym2612_write_reg(0, 0xAD, 0x22);
    expect_ym2612_write_reg(0, 0xA9, 0x84);
    expect_ym2612_write_operator_any_data(CH_SPECIAL_MODE, 0, 0x40);
    midi_receiver_read();

    stub_usb_receive_pitch_bend(TEST_MIDI_CHANNEL_11, 0x4000);
    expect_ym2612_write_reg(0, 0xAD, 0x22);
    expect_ym2612_write_reg(0, 0xA9, 0xD2);
    midi_receiver_read();
}

void test_write_directly_to_ym2612_regs_via_sysex(void** state)
{
    stub_everdrive_as_present();

    const u8 sysExSeq[] = { 0xF0, 0x00, 0x22, 0x77, 0x08, 0x0B, 0x01, 0x01, 0x02, 0xF7 };
    for (u16 i = 0; i < sizeof(sysExSeq); i++) {
        stub_usb_receive_byte(sysExSeq[i]);
    }
    expect_ym2612_write_reg(0, 0xB1, 0x12);
    midi_receiver_read();
}

void test_plays_pcm_sample(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_cc(TEST_MIDI_CHANNEL_1, TEST_CC_ENABLE_DAC, 127);
    expect_ym2612_write_reg(0, 0x2B, 0x80);
    midi_receiver_read();

    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_6, 60, TEST_VOLUME_MAX);
    expect_any(__wrap_SND_PCM_startPlay, sample);
    expect_any(__wrap_SND_PCM_startPlay, len);
    expect_value(__wrap_SND_PCM_startPlay, rate, 1);
    expect_value(__wrap_SND_PCM_startPlay, pan, 0xC0);
    expect_value(__wrap_SND_PCM_startPlay, loop, 0);
    midi_receiver_read();
}

void test_midi_last_note_played_priority_respected_on_fm(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_1, 48, 127);
    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x84);
    expect_ym2612_write_reg(0, 0x28, 0xF0);
    midi_receiver_read();

    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_1, 50, 127);
    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0xD2);
    expect_ym2612_write_reg(0, 0x28, 0xF0);
    midi_receiver_read();

    stub_usb_receive_note_off(TEST_MIDI_CHANNEL_1, 50);
    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x84);
    expect_ym2612_write_reg(0, 0x28, 0xF0);
    midi_receiver_read();
}

void test_midi_last_note_played_remembers_velocity_on_fm(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_1, 48, 100);
    expect_ym2612_write_channel(0, 0x40, 0x27);
    expect_ym2612_write_channel(0, 0x48, 0x04);
    expect_ym2612_write_channel(0, 0x44, 0x24);
    expect_ym2612_write_channel(0, 0x4C, 0x04);
    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x84);
    expect_ym2612_write_reg(0, 0x28, 0xF0);
    midi_receiver_read();

    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_1, 50, 100);
    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0xD2);
    expect_ym2612_write_reg(0, 0x28, 0xF0);
    midi_receiver_read();

    stub_usb_receive_note_off(TEST_MIDI_CHANNEL_1, 50);
    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x84);
    expect_ym2612_write_reg(0, 0x28, 0xF0);
    midi_receiver_read();
}

void test_midi_last_note_played_cleared_when_released_on_fm(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_1, 48, 127);
    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x84);
    expect_ym2612_write_reg(0, 0x28, 0xF0);
    midi_receiver_read();

    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_1, 50, 127);
    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0xD2);
    expect_ym2612_write_reg(0, 0x28, 0xF0);
    midi_receiver_read();

    stub_usb_receive_note_off(TEST_MIDI_CHANNEL_1, 48);
    midi_receiver_read();

    stub_usb_receive_note_off(TEST_MIDI_CHANNEL_1, 50);
    expect_ym2612_write_reg(0, 0x28, 0x0);
    midi_receiver_read();
}

void test_midi_changing_program_retains_pan(void** state)
{
    stub_everdrive_as_present();

    const u8 chan = 0;

    stub_usb_receive_cc(TEST_MIDI_CHANNEL_1, TEST_CC_PAN, 0); // left
    expect_ym2612_write_channel(chan, 0xB4, 0x80); // pan, alg, fb
    midi_receiver_read();

    stub_usb_receive_program(TEST_MIDI_CHANNEL_1, 1);
    expect_ym2612_write_channel_any_data(chan, 0xB0);
    expect_ym2612_write_channel(chan, 0xB4, 0x80); // pan, alg, fb
    expect_ym2612_write_channel_any_data(chan, 0x30);
    expect_ym2612_write_channel_any_data(chan, 0x50);
    expect_ym2612_write_channel_any_data(chan, 0x60);
    expect_ym2612_write_channel_any_data(chan, 0x70);
    expect_ym2612_write_channel_any_data(chan, 0x80);
    expect_ym2612_write_channel_any_data(chan, 0x40);
    expect_ym2612_write_channel_any_data(chan, 0x90);
    expect_ym2612_write_channel_any_data(chan, 0x38);
    expect_ym2612_write_channel_any_data(chan, 0x58);
    expect_ym2612_write_channel_any_data(chan, 0x68);
    expect_ym2612_write_channel_any_data(chan, 0x78);
    expect_ym2612_write_channel_any_data(chan, 0x88);
    expect_ym2612_write_channel_any_data(chan, 0x48);
    expect_ym2612_write_channel_any_data(chan, 0x98);
    expect_ym2612_write_channel_any_data(chan, 0x34);
    expect_ym2612_write_channel_any_data(chan, 0x54);
    expect_ym2612_write_channel_any_data(chan, 0x64);
    expect_ym2612_write_channel_any_data(chan, 0x74);
    expect_ym2612_write_channel_any_data(chan, 0x84);
    expect_ym2612_write_channel_any_data(chan, 0x44);
    expect_ym2612_write_channel_any_data(chan, 0x94);
    expect_ym2612_write_channel_any_data(chan, 0x3C);
    expect_ym2612_write_channel_any_data(chan, 0x5C);
    expect_ym2612_write_channel_any_data(chan, 0x6C);
    expect_ym2612_write_channel_any_data(chan, 0x7C);
    expect_ym2612_write_channel_any_data(chan, 0x8C);
    expect_ym2612_write_channel_any_data(chan, 0x4C);
    expect_ym2612_write_channel_any_data(chan, 0x9C);
    midi_receiver_read();
}

void test_midi_changing_program_retains_volume(void** state)
{
    stub_everdrive_as_present();

    const u8 chan = 0;

    stub_usb_receive_cc(TEST_MIDI_CHANNEL_1, TEST_CC_VOLUME, 0);
    expect_ym2612_write_channel(chan, 0x40, 0x27);
    expect_ym2612_write_channel(chan, 0x48, 0x04);
    expect_ym2612_write_channel(chan, 0x44, 0x24);
    expect_ym2612_write_channel(chan, 0x4C, 0x7F); // output operator mute
    midi_receiver_read();

    stub_usb_receive_program(TEST_MIDI_CHANNEL_1, 1);
    expect_ym2612_write_channel_any_data(chan, 0xB0);
    expect_ym2612_write_channel_any_data(chan, 0xB4);
    expect_ym2612_write_channel_any_data(chan, 0x30);
    expect_ym2612_write_channel_any_data(chan, 0x50);
    expect_ym2612_write_channel_any_data(chan, 0x60);
    expect_ym2612_write_channel_any_data(chan, 0x70);
    expect_ym2612_write_channel_any_data(chan, 0x80);
    expect_ym2612_write_channel(chan, 0x40, 0x21);
    expect_ym2612_write_channel_any_data(chan, 0x90);
    expect_ym2612_write_channel_any_data(chan, 0x38);
    expect_ym2612_write_channel_any_data(chan, 0x58);
    expect_ym2612_write_channel_any_data(chan, 0x68);
    expect_ym2612_write_channel_any_data(chan, 0x78);
    expect_ym2612_write_channel_any_data(chan, 0x88);
    expect_ym2612_write_channel(chan, 0x48, 0x7F); // output operator mute
    expect_ym2612_write_channel_any_data(chan, 0x98);
    expect_ym2612_write_channel_any_data(chan, 0x34);
    expect_ym2612_write_channel_any_data(chan, 0x54);
    expect_ym2612_write_channel_any_data(chan, 0x64);
    expect_ym2612_write_channel_any_data(chan, 0x74);
    expect_ym2612_write_channel_any_data(chan, 0x84);
    expect_ym2612_write_channel(chan, 0x44, 0x7f); // output operator mute
    expect_ym2612_write_channel_any_data(chan, 0x94);
    expect_ym2612_write_channel_any_data(chan, 0x3C);
    expect_ym2612_write_channel_any_data(chan, 0x5C);
    expect_ym2612_write_channel_any_data(chan, 0x6C);
    expect_ym2612_write_channel_any_data(chan, 0x7C);
    expect_ym2612_write_channel_any_data(chan, 0x8C);
    expect_ym2612_write_channel(chan, 0x4C, 0x7f); // output operator mute
    expect_ym2612_write_channel_any_data(chan, 0x9C);
    midi_receiver_read();
}

void test_midi_portamento_glides_note(void** state)
{
    stub_everdrive_as_present();

    stub_usb_receive_cc(TEST_MIDI_CHANNEL_1, TEST_CC_PORTAMENTO_TIME, 95);
    midi_receiver_read();
    stub_usb_receive_cc(TEST_MIDI_CHANNEL_1, TEST_CC_PORTAMENTO_ON, 0x7F);
    midi_receiver_read();

    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_1, 48, 127);
    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x84);
    expect_ym2612_write_reg(0, 0x28, 0xF0);
    midi_receiver_read();

    stub_usb_receive_note_on(TEST_MIDI_CHANNEL_1, 58, 127);
    midi_receiver_read();

    expect_ym2612_write_channel(0, 0xA4, 0x1A);
    expect_ym2612_write_channel(0, 0xA0, 0x87);
    scheduler_vsync();
    scheduler_tick();
    for (u16 i = 0; i < 99; i++) {
        expect_ym2612_write_channel_any_data(0, 0xA4);
        expect_ym2612_write_channel_any_data(0, 0xA0);
        scheduler_vsync();
        scheduler_tick();
    }

    scheduler_vsync();
    scheduler_tick();
    scheduler_vsync();
    scheduler_tick();
}
