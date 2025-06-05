#include "test_midi_sysex.h"
#include "test_midi.h"
#include "mocks/mock_synth.h"
#include "mocks/mock_sram.h"

#define SRAM_PRESET_LENGTH 36
#define SRAM_PRESET_START 32

static void remapChannel(u8 midiChannel, u8 deviceChannel)
{
    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_REMAP, midiChannel, deviceChannel };

    __real_midi_sysex(sequence, sizeof(sequence));
}

void test_midi_sysex_sends_all_notes_off(UNUSED void** state)
{
    const u8 sysExGeneralMidiResetSequence[] = { 0x7E, 0x7F, 0x09, 0x01 };

    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        expect_any_psg_tone();
        expect_any_psg_attenuation();
        __real_midi_note_on(chan, 60, 127);
    }

    for (u8 chan = FM_CH1; chan <= FM_CH6; chan++) {
        expect_synth_note_off(chan);
    }

    for (u8 chan = PSG_CH1; chan <= PSG_NOISE_CH4; chan++) {
        expect_psg_attenuation(chan, PSG_ATTENUATION_SILENCE);
    }

    __real_midi_sysex(sysExGeneralMidiResetSequence, sizeof(sysExGeneralMidiResetSequence));
}

void test_midi_sysex_general_midi_reset_resets_synth_volume(UNUSED void** state)
{
    const u8 sysExGeneralMidiResetSequence[] = { 0x7E, 0x7F, 0x09, 0x01 };

    // Set volume to half-way
    expect_synth_volume(FM_CH1, 64);
    __real_midi_cc(MIDI_CHANNEL_1, CC_VOLUME, 64);

    // Send General MIDI reset
    expect_synth_volume(FM_CH1, MAX_MIDI_VOLUME);
    for (u8 chan = FM_CH1; chan <= FM_CH6; chan++) {
        expect_synth_note_off(chan);
    }
    __real_midi_sysex(sysExGeneralMidiResetSequence, sizeof(sysExGeneralMidiResetSequence));
}

void test_midi_sysex_ignores_unknown_sysex(UNUSED void** state)
{
    const u8 sysExGeneralMidiResetSequence[] = { 0x12 };

    __real_midi_sysex(sysExGeneralMidiResetSequence, sizeof(sysExGeneralMidiResetSequence));
}

void test_midi_sysex_remaps_midi_channel_to_psg(UNUSED void** state)
{
    const u8 FM_CHAN_1 = 0;
    const u8 PSG_TONE_1 = 6;

    remapChannel(UNASSIGNED_MIDI_CHANNEL, FM_CHAN_1);
    remapChannel(MIDI_CHANNEL_1, PSG_TONE_1);

    expect_any_psg_tone_on_channel(PSG_CH1);
    expect_psg_attenuation(PSG_CH1, PSG_ATTENUATION_LOUDEST);

    __real_midi_note_on(MIDI_CHANNEL_1, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);
}

void test_midi_sysex_remaps_midi_channel_to_fm(UNUSED void** state)
{
    const u8 FM_CHAN_2 = 1;
    const u8 FM_CHAN_1 = 0;

    remapChannel(UNASSIGNED_MIDI_CHANNEL, FM_CHAN_1);
    remapChannel(MIDI_CHANNEL_1, FM_CHAN_2);

    expect_value(__wrap_synth_pitch, channel, 1);
    expect_any(__wrap_synth_pitch, octave);
    expect_any(__wrap_synth_pitch, freqNumber);
    expect_synth_volume_any();
    expect_value(__wrap_synth_note_on, channel, 1);

    __real_midi_note_on(0, 60, MAX_MIDI_VOLUME);
}

void test_midi_sysex_unassigns_midi_channel(UNUSED void** state)
{
    remapChannel(0, 0x7F);

    __real_midi_note_on(0, 60, MAX_MIDI_VOLUME);
}

void test_midi_sysex_does_nothing_for_empty_payload(UNUSED void** state)
{
    const u16 length = 0;
    u8 seq[1];

    __real_midi_sysex(seq, length);
}

void test_midi_sysex_handles_incomplete_channel_mapping_command(UNUSED void** state)
{
    const u8 sequence[]
        = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID, SYSEX_COMMAND_REMAP };

    __real_midi_sysex(sequence, 4);
}

void test_midi_sysex_enables_dynamic_channel_mode(UNUSED void** state)
{
    const u8 sequence[] = {
        SYSEX_MANU_EXTENDED,
        SYSEX_MANU_REGION,
        SYSEX_MANU_ID,
        SYSEX_COMMAND_DYNAMIC,
        SYSEX_DYNAMIC_ENABLED,
    };

    __real_midi_sysex(sequence, sizeof(sequence));

    // Initial note
    expect_synth_pitch(0, 4, 0x284);
    expect_synth_volume_any();
    expect_value(__wrap_synth_note_on, channel, 0);
    __real_midi_note_on(0, 60, MAX_MIDI_VOLUME);

    // Second note
    expect_synth_pitch(1, 4, 0x2a9);
    expect_synth_volume_any();
    expect_value(__wrap_synth_note_on, channel, 1);
    __real_midi_note_on(0, 61, MAX_MIDI_VOLUME);
}

void test_midi_sysex_sets_mapping_mode_to_auto(UNUSED void** state)
{
    const u8 sequence[] = {
        SYSEX_MANU_EXTENDED,
        SYSEX_MANU_REGION,
        SYSEX_MANU_ID,
        SYSEX_COMMAND_DYNAMIC,
        SYSEX_DYNAMIC_AUTO,
    };
    __real_midi_sysex(sequence, sizeof(sequence));

    const u8 sysExGeneralMidiResetSequence[] = { 0x7E, 0x7F, 0x09, 0x01 };
    for (u16 ch = 0; ch < MAX_FM_CHANS; ch++) {
        expect_value(__wrap_synth_note_off, channel, ch);
    }
    __real_midi_sysex(sysExGeneralMidiResetSequence, sizeof(sysExGeneralMidiResetSequence));

    // Initial note
    expect_synth_pitch(0, 4, 0x284);
    expect_synth_volume_any();
    expect_value(__wrap_synth_note_on, channel, 0);
    __real_midi_note_on(0, 60, MAX_MIDI_VOLUME);

    // Second note
    expect_synth_pitch(1, 4, 0x2a9);
    expect_synth_volume_any();
    expect_value(__wrap_synth_note_on, channel, 1);
    __real_midi_note_on(0, 61, MAX_MIDI_VOLUME);
}

void test_midi_sysex_disables_fm_parameter_CCs(UNUSED void** state)
{
    const u8 sequence[] = {
        SYSEX_MANU_EXTENDED,
        SYSEX_MANU_REGION,
        SYSEX_MANU_ID,
        SYSEX_COMMAND_NON_GENERAL_MIDI_CCS,
        SYSEX_NON_GENERAL_MIDI_CCS_DISABLED,
    };

    __real_midi_sysex(sequence, sizeof(sequence));

    const u8 nonGeneralMidiCCs[]
        = { CC_GENMDM_FM_ALGORITHM, CC_GENMDM_FM_FEEDBACK, CC_GENMDM_TOTAL_LEVEL_OP1,
              CC_GENMDM_DETUNE_OP4, CC_GENMDM_RATE_SCALING_OP1, CC_GENMDM_RELEASE_RATE_OP4,
              CC_GENMDM_AMPLITUDE_MODULATION_OP1, CC_GENMDM_STEREO, CC_GENMDM_SSG_EG_OP1,
              CC_GENMDM_SSG_EG_OP4, CC_GENMDM_GLOBAL_LFO_FREQUENCY, CC_GENMDM_ENABLE_DAC };

    for (u16 i = 0; i < sizeof(nonGeneralMidiCCs) / sizeof(u8); i++) {
        u8 cc = nonGeneralMidiCCs[i];
        __real_midi_cc(0, cc, 1);
    }
}

void test_midi_sysex_loads_psg_envelope(UNUSED void** state)
{
    mock_log_enable_checks();

    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_LOAD_PSG_ENVELOPE, 0x01, 0x01 };

    const u8 eef[] = { 0x11, EEF_END };
    const u8 eefLength = sizeof(eef) / sizeof(u8);

    expect_memory(__wrap_midi_psg_load_envelope, eef, eef, eefLength);

    expect_memory(__wrap_log_info, fmt, "Loaded User Defined Envelope", 29);

    __real_midi_sysex(sequence, sizeof(sequence));
}

void test_midi_sysex_inverts_total_level_values(UNUSED void** state)
{
    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_INVERT_TOTAL_LEVEL, 0x01 };

    __real_midi_sysex(sequence, sizeof(sequence));

    expect_synth_operator_total_level(0, 0, 126);
    __real_midi_cc(0, CC_GENMDM_TOTAL_LEVEL_OP1, 1);
}

void test_midi_sysex_sets_original_total_level_values(UNUSED void** state)
{
    const u8 invert_sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_INVERT_TOTAL_LEVEL, 0x01 };

    __real_midi_sysex(invert_sequence, sizeof(invert_sequence));
    expect_synth_operator_total_level(0, 0, 126);
    __real_midi_cc(0, CC_GENMDM_TOTAL_LEVEL_OP1, 1);

    const u8 original_sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_INVERT_TOTAL_LEVEL, 0x00 };

    __real_midi_sysex(original_sequence, sizeof(original_sequence));
    expect_synth_operator_total_level(0, 0, 126);
    __real_midi_cc(0, CC_GENMDM_TOTAL_LEVEL_OP1, 126);
}

void test_midi_sysex_writes_directly_to_ym2612_regs_part_0(UNUSED void** state)
{
    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_WRITE_YM2612_REG_PART_0, 0x0B, 0x01, 0x01, 0x02 };

    expect_synth_direct_write_ym2612(0, 0xB1, 0x12);
    __real_midi_sysex(sequence, sizeof(sequence));
}

void test_midi_sysex_writes_directly_to_ym2612_regs_part_1(UNUSED void** state)
{
    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_WRITE_YM2612_REG_PART_1, 0x0B, 0x01, 0x01, 0x02 };

    expect_synth_direct_write_ym2612(1, 0xB1, 0x12);
    __real_midi_sysex(sequence, sizeof(sequence));
}

void test_midi_sysex_ignores_incorrect_length_ym2612_direct_writes(UNUSED void** state)
{
    const u8 badSeq1[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_WRITE_YM2612_REG_PART_0, 0x0B, 0x01, 0x01, 0x02, 0x02 };
    __real_midi_sysex(badSeq1, sizeof(badSeq1));

    const u8 badSeq2[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_WRITE_YM2612_REG_PART_0, 0x0B, 0x01, 0x01 };
    __real_midi_sysex(badSeq2, sizeof(badSeq2));

    const u8 badSeq3[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_WRITE_YM2612_REG_PART_0 };
    __real_midi_sysex(badSeq3, sizeof(badSeq3));
}

#define STORE_PROGRAM_MESSAGE_LENGTH (10 + (MAX_FM_OPERATORS * 11))

static void create_store_program_message(u8* msg, u8 type, u8 program, FmPreset* fmPreset)
{
    msg[0] = SYSEX_MANU_EXTENDED;
    msg[1] = SYSEX_MANU_REGION;
    msg[2] = SYSEX_MANU_ID;
    msg[3] = SYSEX_COMMAND_STORE_PROGRAM;
    msg[4] = type;
    msg[5] = program;
    msg[6] = fmPreset->algorithm;
    msg[7] = fmPreset->feedback;
    msg[8] = fmPreset->ams;
    msg[9] = fmPreset->fms;
    for (u16 i = 0; i < MAX_FM_OPERATORS; i++) {
        msg[10 + i * 11] = fmPreset->operators[i].multiple;
        msg[11 + i * 11] = fmPreset->operators[i].detune;
        msg[12 + i * 11] = fmPreset->operators[i].attackRate;
        msg[13 + i * 11] = fmPreset->operators[i].rateScaling;
        msg[14 + i * 11] = fmPreset->operators[i].decayRate;
        msg[15 + i * 11] = fmPreset->operators[i].amplitudeModulation;
        msg[16 + i * 11] = fmPreset->operators[i].sustainLevel;
        msg[17 + i * 11] = fmPreset->operators[i].sustainRate;
        msg[18 + i * 11] = fmPreset->operators[i].releaseRate;
        msg[19 + i * 11] = fmPreset->operators[i].totalLevel;
        msg[20 + i * 11] = fmPreset->operators[i].ssgEg;
    }
}

void test_midi_sysex_stores_program(UNUSED void** state)
{
    mock_log_enable_checks();

    const u8 program = 0x05;

    FmPreset fmPreset = { 0 };
    memcpy(&fmPreset, &TEST_M_BANK_0_INST_0_GRANDPIANO, sizeof(FmChannel));
    fmPreset.algorithm = 0x07;

    u8 msg[STORE_PROGRAM_MESSAGE_LENGTH];
    create_store_program_message(msg, STORE_PROGRAM_TYPE_FM, program, &fmPreset);

    expect_log_info("Prg %d: FM preset stored");
    __real_midi_sysex(msg, sizeof(msg));

    expect_synth_preset(FM_CH1, &fmPreset);
    __real_midi_program(MIDI_CHANNEL_1, program);

    const u8 EXPECTED_SRAM_DATA[SRAM_PRESET_LENGTH]
        = { /* magic number */ 0x9E, 0x1D, /* version */ 0x01,
              /* preset */ 0xE0, 0x00, 0x11, 0xA4, 0xE7, 0x20, 0xA7, 0x00, 0x4D, 0x85, 0x26, 0x4B,
              0xA4, 0x00, 0x2F, 0xFE, 0xE9, 0x78, 0x84, 0x00, 0x17, 0xB8, 0x8A, 0x23, 0x02, 0x00,
              /* reserved */ 0x00, 0x00, 0x00, 0x00, 0x00, /* checksum */ 0x40, 0x47 };

    u16 offset = SRAM_PRESET_START + (program * SRAM_PRESET_LENGTH);
    assert_memory_equal(mock_sram_data(offset), EXPECTED_SRAM_DATA, SRAM_PRESET_LENGTH);
}

void test_midi_sysex_logs_warning_if_program_store_length_is_incorrect(UNUSED void** state)
{
    mock_log_enable_checks();

    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_STORE_PROGRAM, 0x00, 0x05 };

    expect_log_warn("Sysex %02X: Invalid length: %d (!= %d)");
    __real_midi_sysex(sequence, sizeof(sequence));
}

void test_midi_sysex_logs_warning_if_program_store_type_is_incorrect(UNUSED void** state)
{
    mock_log_enable_checks();

    const u8 sequence[STORE_PROGRAM_MESSAGE_LENGTH] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION,
        SYSEX_MANU_ID, SYSEX_COMMAND_STORE_PROGRAM, 0x01, 0x05 };

    expect_log_warn("Invalid store program type: %d");
    __real_midi_sysex(sequence, sizeof(sequence));
}

void test_midi_sysex_clears_program(UNUSED void** state)
{
    const u8 program = 0x01;

    FmPreset fmPreset = { 0 };
    memcpy(&fmPreset, &TEST_M_BANK_0_INST_0_GRANDPIANO, sizeof(FmPreset));
    fmPreset.algorithm = 0x07;

    u8 msg[STORE_PROGRAM_MESSAGE_LENGTH];
    create_store_program_message(msg, STORE_PROGRAM_TYPE_FM, program, &fmPreset);
    __real_midi_sysex(msg, sizeof(msg));

    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_CLEAR_PROGRAM, STORE_PROGRAM_TYPE_FM, program };
    mock_log_enable_checks();
    expect_log_info("Prg %d: FM preset cleared");
    __real_midi_sysex(sequence, sizeof(sequence));

    expect_synth_preset(FM_CH1, &TEST_M_BANK_0_INST_1_BRIGHTPIANO);
    __real_midi_program(MIDI_CHANNEL_1, program);

    u16 offset = SRAM_PRESET_START + SRAM_PRESET_LENGTH * program;
    const u8 CLEARED_MAGIC_NUMBER[2] = { 0, 0 };
    assert_memory_equal(mock_sram_data(offset), CLEARED_MAGIC_NUMBER, 2);
}

void test_midi_sysex_logs_warning_if_program_clear_length_is_incorrect(UNUSED void** state)
{
    mock_log_enable_checks();

    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_CLEAR_PROGRAM, STORE_PROGRAM_TYPE_FM };

    expect_log_warn("Sysex %02X: Invalid length: %d (!= %d)");
    __real_midi_sysex(sequence, sizeof(sequence));
}

void test_midi_sysex_logs_warning_if_program_clear_type_is_incorrect(UNUSED void** state)
{
    mock_log_enable_checks();

    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_CLEAR_PROGRAM, 0x01, 0x01 };

    expect_log_warn("Invalid clear program type: %d");
    __real_midi_sysex(sequence, sizeof(sequence));
}

void test_midi_sysex_clears_all_programs(UNUSED void** state)
{
    const u8 program1 = 0x00;
    const u8 program2 = 0x01;

    FmPreset fmPreset = { 0 };
    u8 msg[STORE_PROGRAM_MESSAGE_LENGTH];

    memcpy(&fmPreset, &TEST_M_BANK_0_INST_0_GRANDPIANO, sizeof(FmPreset));
    fmPreset.algorithm = 0x07;
    create_store_program_message(msg, STORE_PROGRAM_TYPE_FM, program1, &fmPreset);
    __real_midi_sysex(msg, sizeof(msg));

    memcpy(&fmPreset, &TEST_M_BANK_0_INST_1_BRIGHTPIANO, sizeof(FmPreset));
    fmPreset.algorithm = 0x04;
    create_store_program_message(msg, STORE_PROGRAM_TYPE_FM, program2, &fmPreset);
    __real_midi_sysex(msg, sizeof(msg));

    mock_log_enable_checks();

    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_CLEAR_ALL_PROGRAMS, STORE_PROGRAM_TYPE_FM };

    expect_log_info("All FM presets cleared");
    __real_midi_sysex(sequence, sizeof(sequence));

    expect_synth_preset(FM_CH1, &TEST_M_BANK_0_INST_1_BRIGHTPIANO);
    __real_midi_program(MIDI_CHANNEL_1, program2);

    expect_synth_preset(FM_CH1, &TEST_M_BANK_0_INST_0_GRANDPIANO);
    __real_midi_program(MIDI_CHANNEL_1, program1);
}

void test_midi_sysex_logs_warning_if_clear_all_programs_type_is_incorrect(UNUSED void** state)
{
    mock_log_enable_checks();

    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_CLEAR_ALL_PROGRAMS, 0x01 };

    expect_log_warn("Invalid clear all programs type: %d");
    __real_midi_sysex(sequence, sizeof(sequence));
}

void test_midi_loads_presets_from_sram(UNUSED void** state)
{
    // store a preset to SRAM
    const u8 program = 0x01;

    FmPreset fmPreset;
    memcpy(&fmPreset, &TEST_M_BANK_0_INST_0_GRANDPIANO, sizeof(FmPreset));
    fmPreset.algorithm = 0x07;

    u8 msg[STORE_PROGRAM_MESSAGE_LENGTH];
    create_store_program_message(msg, STORE_PROGRAM_TYPE_FM, program, &fmPreset);
    __real_midi_sysex(msg, sizeof(msg));

    // reset (wipes all presets from RAM)
    mock_log_enable_checks();
    mock_synth_disable_checks();
    expect_log_info("Loaded %d FM presets");
    __real_midi_reset();
    mock_synth_enable_checks();

    // check that the preset was loaded
    expect_synth_preset(FM_CH1, &fmPreset);
    __real_midi_program(MIDI_CHANNEL_1, program);
}

void test_midi_sysex_does_not_display_loaded_msg_if_no_presets_are_loaded(UNUSED void** state)
{
    mock_log_enable_checks();
    mock_synth_disable_checks();
    __real_midi_reset();
}

void test_midi_cc_stores_program(UNUSED void** state)
{
    const u8 program = 0x01;

    expect_synth_algorithm(FM_CH1, 0x06);
    __real_midi_cc(MIDI_CHANNEL_1, CC_GENMDM_FM_ALGORITHM, 111);

    FmPreset fmPreset;
    memcpy(&fmPreset, &TEST_M_BANK_0_INST_0_GRANDPIANO, sizeof(FmPreset));
    fmPreset.algorithm = 0x06;

    mock_log_enable_checks();
    expect_synth_extract_preset(FM_CH1, &fmPreset);
    expect_log_info("Prg %d: FM preset stored");
    __real_midi_cc(MIDI_CHANNEL_1, CC_STORE_PROGRAM, program);

    expect_synth_algorithm(FM_CH1, 0x00);
    __real_midi_cc(MIDI_CHANNEL_1, CC_GENMDM_FM_ALGORITHM, 0);

    expect_synth_preset(FM_CH1, &fmPreset);
    __real_midi_program(MIDI_CHANNEL_1, program);

    // check that the preset was stored to SRAM
    const u8 EXPECTED_SRAM_DATA[SRAM_PRESET_LENGTH]
        = { /* magic number */ 0x9E, 0x1D, /* version */ 0x01,
              /* preset */ 0xC0, 0x00, 0x11, 0xA4, 0xE7, 0x20, 0xA7, 0x00, 0x4D, 0x85, 0x26, 0x4B,
              0xA4, 0x00, 0x2F, 0xFE, 0xE9, 0x78, 0x84, 0x00, 0x17, 0xB8, 0x8A, 0x23, 0x02, 0x00,
              /* reserved */ 0x00, 0x00, 0x00, 0x00, 0x00, /* checksum */ 0x87, 0x77 };

    u16 offset = SRAM_PRESET_START + (program * SRAM_PRESET_LENGTH);
    assert_memory_equal(mock_sram_data(offset), EXPECTED_SRAM_DATA, SRAM_PRESET_LENGTH);
}

void test_midi_sysex_validates_remap_channel_bounds(UNUSED void** state)
{
    mock_log_enable_checks();

    // Test 1: Invalid device channel (>= DEV_CHANS)
    const u8 invalid_dev_chan_seq[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_REMAP, 0x00, 0xFF };

    expect_log_warn("Invalid device channel: %d");
    __real_midi_sysex(invalid_dev_chan_seq, sizeof(invalid_dev_chan_seq));

    // Test 2: Another invalid device channel (just over the limit)
    const u8 boundary_dev_chan_seq[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_REMAP, 0x00, 0x0D };

    expect_log_warn("Invalid device channel: %d");
    __real_midi_sysex(boundary_dev_chan_seq, sizeof(boundary_dev_chan_seq));

    // Test 3: Invalid MIDI channel (>= MIDI_CHANNELS)
    const u8 invalid_midi_chan_seq[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_REMAP, 0x10, 0x00 };

    expect_log_warn("Invalid MIDI channel: %d");
    __real_midi_sysex(invalid_midi_chan_seq, sizeof(invalid_midi_chan_seq));

    // Test 4: Another invalid MIDI channel
    const u8 extreme_midi_chan_seq[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_REMAP, 0xFF, 0x00 };

    expect_log_warn("Invalid MIDI channel: %d");
    __real_midi_sysex(extreme_midi_chan_seq, sizeof(extreme_midi_chan_seq));
}
