#include "test_e2e.h"
#include "comm/comm.h"
#include "envelopes.h"
#include "midi.h"
#include "midi_rx.h"
#include "presets.h"
#include "scheduler.h"
#include "mocks/mock_ym2612.h"
#include "mocks/mock_synth.h"
#include "mocks/mock_comm.h"
#include "mocks/mock_sgdk.h"
#include "mocks/mock_psg.h"
#include "ym2612_regs.h"
#include "test_helpers.h"

int test_e2e_setup(void** state)
{
    mock_sgdk_disable_checks();
    mock_ym2612_disable_checks();
    mock_synth_disable_checks();

    scheduler_init();
    comm_reset_counts();
    stub_megawifi_as_not_present();
    comm_init();
    midi_init(M_BANK_0, P_BANK_0, ENVELOPES);

    mock_sgdk_enable_checks();
    mock_ym2612_enable_checks();
    mock_synth_enable_checks();
    return 0;
}

void test_midi_note_on_event_sent_to_ym2612(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_note_on(MIDI_CHANNEL_1, 48, MIDI_VELOCITY_MAX);
    expect_ym2612_write_frequency(YM_CH1, 0x1A84);
    expect_ym2612_note_on(YM_CH1);
    midi_rx_read();
}

void test_polyphonic_midi_sent_to_separate_ym2612_channels(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_cc(MIDI_CHANNEL_1, CC_GENMDM_POLYPHONIC_MODE, POLYPHONIC_ON);
    midi_rx_read();

    stub_usb_receive_note_on(MIDI_CHANNEL_1, 48, MIDI_VELOCITY_MAX);
    expect_ym2612_write_frequency(YM_CH1, 0x1A84);
    expect_ym2612_note_on(YM_CH1);
    midi_rx_read();

    stub_usb_receive_note_on(MIDI_CHANNEL_1, 49, MIDI_VELOCITY_MAX);
    expect_ym2612_write_frequency(YM_CH2, 0x1AA9);
    expect_ym2612_note_on(YM_CH2);
    midi_rx_read();
}

void test_psg_audible_if_note_on_event_triggered(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_note_on(MIDI_CHANNEL_PSG_1, 60, MIDI_VELOCITY_MAX);
    expect_any_psg_tone();
    expect_any_psg_attenuation();
    midi_rx_read();
}

void test_psg_not_audible_if_midi_channel_volume_set_and_there_is_no_note_on_event(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_cc(MIDI_CHANNEL_PSG_1, CC_VOLUME, MIDI_VOLUME_MAX);
    midi_rx_read();
}

void test_general_midi_reset_sysex_stops_all_notes(void** state)
{
    stub_everdrive_as_present();

    const u8 noteOnKey = 48;

    // FM note
    stub_usb_receive_note_on(MIDI_CHANNEL_1, noteOnKey, MIDI_VELOCITY_MAX);
    expect_ym2612_write_frequency(YM_CH1, 0x1A84);
    expect_ym2612_note_on(YM_CH1);
    midi_rx_read();

    // PSG note
    stub_usb_receive_note_on(MIDI_CHANNEL_PSG_1, noteOnKey, MIDI_VELOCITY_MAX);
    expect_psg_tone(PSG_CH1, 0x357);
    expect_psg_attenuation(PSG_CH1, 0);
    midi_rx_read();

    // Send General MIDI reset
    const u8 sysExGeneralMidiResetSequence[] = { 0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7 };
    for (u16 i = 0; i < sizeof(sysExGeneralMidiResetSequence); i++) {
        stub_usb_receive_byte(sysExGeneralMidiResetSequence[i]);
    }
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        expect_ym2612_note_off(chan);
    }
    expect_psg_attenuation(PSG_CH1, 0xF);
    midi_rx_read();
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
    midi_rx_read();

    remapChannel(MIDI_CHANNEL_1, DEVICE_PSG_1);
    midi_rx_read();

    stub_usb_receive_note_on(MIDI_CHANNEL_1, 48, MIDI_VELOCITY_MAX);
    expect_psg_tone(PSG_CH1, 0x357);
    expect_psg_attenuation(PSG_CH1, 0);
    midi_rx_read();
}

void test_set_device_for_midi_channel_1_to_psg()
{
    stub_everdrive_as_present();
    stub_usb_receive_cc(MIDI_CHANNEL_1, CC_GENMDM_POLYPHONIC_MODE, POLYPHONIC_ON);
    midi_rx_read();

    stub_usb_receive_cc(MIDI_CHANNEL_1, CC_DEVICE_SELECT, DEVICE_SELECT_PSG);
    midi_rx_read();

    stub_usb_receive_note_on(MIDI_CHANNEL_1, 48, MIDI_VELOCITY_MAX);
    expect_psg_tone(PSG_CH1, 0x357);
    expect_psg_attenuation(PSG_CH1, 0);
    midi_rx_read();
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

    midi_rx_read();
}

void test_loads_psg_envelope()
{
    const u8 sysExPingSequence[] = { SYSEX_START, SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION,
        SYSEX_MANU_ID, SYSEX_COMMAND_LOAD_PSG_ENVELOPE, 0x06, 0x06, SYSEX_END };

    stub_everdrive_as_present();
    for (u16 i = 0; i < sizeof(sysExPingSequence); i++) {
        stub_usb_receive_byte(sysExPingSequence[i]);
    }
    midi_rx_read();

    stub_usb_receive_note_on(MIDI_CHANNEL_PSG_1, 60, MIDI_VELOCITY_MAX);
    expect_psg_tone(PSG_CH1, 0x17c);
    expect_psg_attenuation(PSG_CH1, 6);
    midi_rx_read();
}

void test_enables_ch3_special_mode(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_cc(MIDI_CHANNEL_1, CC_GENMDM_CH3_SPECIAL_MODE, SPECIAL_MODE_ON);
    expect_ym2612_write_reg(0, YM_CH3_MODE, 0x40);
    midi_rx_read();
}

void test_sets_separate_ch3_operator_frequencies(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_cc(MIDI_CHANNEL_1, CC_GENMDM_CH3_SPECIAL_MODE, SPECIAL_MODE_ON);
    expect_ym2612_write_reg(0, YM_CH3_MODE, 0x40);
    midi_rx_read();

    stub_usb_receive_cc(MIDI_CHANNEL_3, CC_GENMDM_FM_ALGORITHM, 0x7F); // alg 7
    expect_ym2612_write_reg(0, 0xB2, 0x7);
    midi_rx_read();

    const u8 upperRegs[] = { 0xAD, 0xAE, 0xAC }; // ops 0, 1, 2
    const u8 lowerRegs[] = { 0xA9, 0xAA, 0xA8 };
    const u8 tlValues[] = { 0x27, 0x04, 0x24 };

    for (u8 op = 0; op < 3; op++) {
        stub_usb_receive_note_on(MIDI_CHANNEL_11 + op, 60, 120);
        expect_ym2612_write_reg(0, upperRegs[op], 0x22);
        expect_ym2612_write_reg(0, lowerRegs[op], 0x84);
        expect_ym2612_write_operator(CH3_SPECIAL_MODE, op, 0x40, tlValues[op]);
        midi_rx_read();
    }
}

void test_pitch_bends_ch3_special_mode_operators(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_cc(MIDI_CHANNEL_1, CC_GENMDM_CH3_SPECIAL_MODE, SPECIAL_MODE_ON);
    expect_ym2612_write_reg(0, YM_CH3_MODE, 0x40);
    midi_rx_read();

    stub_usb_receive_cc(MIDI_CHANNEL_3, CC_GENMDM_FM_ALGORITHM, 0x7F); // alg 7
    expect_ym2612_write_reg(0, YM_REG(YM_BASE_ALGORITHM_FEEDBACK, YM_CH3), 0x7);
    midi_rx_read();

    stub_usb_receive_note_on(MIDI_CHANNEL_11, 60, 120);
    expect_ym2612_write_reg(0, YM_CH3SM_OP1_FREQ_MSB_BLK, 0x22);
    expect_ym2612_write_reg(0, YM_CH3SM_OP1_FREQ_LSB, 0x84);
    expect_ym2612_write_operator_any_data(CH3_SPECIAL_MODE, 0, 0x40);
    midi_rx_read();

    stub_usb_receive_pitch_bend(MIDI_CHANNEL_11, 0x4000);
    expect_ym2612_write_reg(0, YM_CH3SM_OP1_FREQ_MSB_BLK, 0x22);
    expect_ym2612_write_reg(0, YM_CH3SM_OP1_FREQ_LSB, 0xD2);
    midi_rx_read();
}

void test_write_directly_to_ym2612_regs_via_sysex(void** state)
{
    stub_everdrive_as_present();

    const u8 sysExSeq[] = { 0xF0, 0x00, 0x22, 0x77, 0x08, 0x0B, 0x01, 0x01, 0x02, 0xF7 };
    for (u16 i = 0; i < sizeof(sysExSeq); i++) {
        stub_usb_receive_byte(sysExSeq[i]);
    }
    expect_ym2612_write_reg(0, 0xB1, 0x12);
    midi_rx_read();
}

void test_plays_pcm_sample(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_cc(MIDI_CHANNEL_1, CC_GENMDM_ENABLE_DAC, 127);
    expect_ym2612_write_reg(0, YM_DAC_ENABLE, 0x80);
    midi_rx_read();

    stub_usb_receive_note_on(MIDI_CHANNEL_6, 60, MIDI_VOLUME_MAX);
    expect_any(__wrap_SND_PCM_startPlay, sample);
    expect_any(__wrap_SND_PCM_startPlay, len);
    expect_value(__wrap_SND_PCM_startPlay, rate, 1);
    expect_value(__wrap_SND_PCM_startPlay, pan, 0xC0);
    expect_value(__wrap_SND_PCM_startPlay, loop, 0);
    midi_rx_read();
}

void test_midi_last_note_played_priority_respected_on_fm(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_note_on(MIDI_CHANNEL_1, 48, MIDI_VELOCITY_MAX);
    expect_ym2612_write_frequency(YM_CH1, 0x1A84);
    expect_ym2612_note_on(YM_CH1);
    midi_rx_read();

    stub_usb_receive_note_on(MIDI_CHANNEL_1, 50, MIDI_VELOCITY_MAX);
    expect_ym2612_write_frequency(YM_CH1, 0x1AD2);
    expect_ym2612_note_on(YM_CH1);
    midi_rx_read();

    stub_usb_receive_note_off(MIDI_CHANNEL_1, 50);
    expect_ym2612_write_frequency(YM_CH1, 0x1A84);
    expect_ym2612_note_on(YM_CH1);
    midi_rx_read();
}

void test_midi_last_note_played_remembers_velocity_on_fm(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_note_on(MIDI_CHANNEL_1, 48, 100);
    expect_ym2612_write_operator_volumes(0, ((u8[]) { 0x27, 0x04, 0x24, 0x04 }), 4);
    expect_ym2612_write_frequency(YM_CH1, 0x1A84);
    expect_ym2612_note_on(YM_CH1);
    midi_rx_read();

    stub_usb_receive_note_on(MIDI_CHANNEL_1, 50, 100);
    expect_ym2612_write_frequency(YM_CH1, 0x1AD2);
    expect_ym2612_note_on(YM_CH1);
    midi_rx_read();

    stub_usb_receive_note_off(MIDI_CHANNEL_1, 50);
    expect_ym2612_write_frequency(YM_CH1, 0x1A84);
    expect_ym2612_note_on(YM_CH1);
    midi_rx_read();
}

void test_midi_last_note_played_cleared_when_released_on_fm(void** state)
{
    stub_everdrive_as_present();
    stub_usb_receive_note_on(MIDI_CHANNEL_1, 48, MIDI_VELOCITY_MAX);
    expect_ym2612_write_frequency(YM_CH1, 0x1A84);
    expect_ym2612_note_on(YM_CH1);
    midi_rx_read();

    stub_usb_receive_note_on(MIDI_CHANNEL_1, 50, MIDI_VELOCITY_MAX);
    expect_ym2612_write_frequency(YM_CH1, 0x1AD2);
    expect_ym2612_note_on(YM_CH1);
    midi_rx_read();

    stub_usb_receive_note_off(MIDI_CHANNEL_1, 48);
    midi_rx_read();

    stub_usb_receive_note_off(MIDI_CHANNEL_1, 50);
    expect_ym2612_note_off(YM_CH1);
    midi_rx_read();
}

void test_midi_changing_program_retains_pan(void** state)
{
    stub_everdrive_as_present();

    const u8 chan = YM_CH1;

    stub_usb_receive_cc(MIDI_CHANNEL_1, CC_PAN, 0); // left
    expect_ym2612_write_channel(chan, YM_BASE_STEREO_AMS_PMS, 0x80); // pan, alg, fb
    midi_rx_read();

    stub_usb_receive_program(MIDI_CHANNEL_1, 1);
    expect_ym2612_write_channel_any_data(chan, YM_BASE_ALGORITHM_FEEDBACK);
    expect_ym2612_write_channel(chan, YM_BASE_STEREO_AMS_PMS, 0x80); // pan, alg, fb

    for (u8 op = YM_OP1; op <= YM_OP4; op++) {
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_MULTIPLE_DETUNE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_ATTACK_RATE_SCALING_RATE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_DECAY_RATE_AM_ENABLE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_SUSTAIN_RATE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_RELEASE_RATE_SUSTAIN_LEVEL, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_TOTAL_LEVEL, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_SSG_EG, op));
    }
    midi_rx_read();
}

void test_midi_changing_program_retains_volume(void** state)
{
    stub_everdrive_as_present();

    const u8 chan = YM_CH1;

    stub_usb_receive_cc(MIDI_CHANNEL_1, CC_VOLUME, 0);
    expect_ym2612_write_operator_volumes(
        chan, ((u8[]) { 0x27, 0x04, 0x24, YM_TOTAL_LEVEL_SILENCE }), 4);
    midi_rx_read();

    stub_usb_receive_program(MIDI_CHANNEL_1, 1);
    expect_ym2612_write_channel_any_data(chan, YM_BASE_ALGORITHM_FEEDBACK);
    expect_ym2612_write_channel_any_data(chan, YM_BASE_STEREO_AMS_PMS);

    for (u8 op = YM_OP1; op <= YM_OP4; op++) {
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_MULTIPLE_DETUNE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_ATTACK_RATE_SCALING_RATE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_DECAY_RATE_AM_ENABLE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_SUSTAIN_RATE, op));
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_RELEASE_RATE_SUSTAIN_LEVEL, op));
        expect_ym2612_write_channel(
            chan, YM_REG3(YM_BASE_TOTAL_LEVEL, op), op == YM_OP1 ? 0x21 : YM_TOTAL_LEVEL_SILENCE);
        expect_ym2612_write_channel_any_data(chan, YM_REG3(YM_BASE_SSG_EG, op));
    }
    midi_rx_read();
}

void test_midi_portamento_glides_note(void** state)
{
    stub_everdrive_as_present();

    stub_usb_receive_cc(MIDI_CHANNEL_1, CC_PORTAMENTO_TIME_MSB, 95);
    midi_rx_read();
    stub_usb_receive_cc(MIDI_CHANNEL_1, CC_PORTAMENTO_ENABLE, 0x7F);
    midi_rx_read();

    stub_usb_receive_note_on(MIDI_CHANNEL_1, 48, MIDI_VELOCITY_MAX);
    expect_ym2612_write_frequency(YM_CH1, 0x1A84);
    expect_ym2612_note_on(YM_CH1);
    midi_rx_read();

    stub_usb_receive_note_on(MIDI_CHANNEL_1, 58, MIDI_VELOCITY_MAX);
    midi_rx_read();

    expect_ym2612_write_frequency(YM_CH1, 0x1A87);
    scheduler_vsync();
    scheduler_tick();
    for (u16 i = 0; i < 99; i++) {
        expect_ym2612_write_frequency_any_data(YM_CH1);
        scheduler_vsync();
        scheduler_tick();
    }

    scheduler_vsync();
    scheduler_tick();
    scheduler_vsync();
    scheduler_tick();
}
