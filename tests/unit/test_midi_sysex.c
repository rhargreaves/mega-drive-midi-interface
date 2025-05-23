#include "test_midi_sysex.h"
#include "test_midi.h"
#include "mocks/mock_synth.h"

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

void test_midi_sysex_stores_program(UNUSED void** state)
{
    /*
typedef struct Operator {
    u8 multiple : 4;
    u8 detune : 3;
    u8 attackRate : 5;
    u8 rateScaling : 2;
    u8 decayRate : 5;
    u8 amplitudeModulation : 1;
    u8 sustainLevel : 4;
    u8 sustainRate : 5;
    u8 releaseRate : 4;
    u8 totalLevel : 7;
    u8 ssgEg : 4;
} Operator;

typedef struct FmChannel {
    u8 algorithm : 3;
    u8 feedback : 3;
    u8 stereo : 2;
    u8 ams : 2;
    u8 fms : 3;
    u8 octave : 3;
    u16 freqNumber : 11;
    Operator operators[MAX_FM_OPERATORS];
} FmChannel;
*/

    /*
    const u8 payload[] = {
        0x00, // algorithm
        0x00, // feedback
        0x00, // stereo
        0x00, // ams
        0x00, // fms
    }; // 5 bytes

    const u8 op1[] = {
        0x00, // op1 multiple
        0x00, // op1 detune
        0x00, // op1 attack rate
        0x00, // op1 rate scaling
        0x00, // op1 decay rate
        0x00, // op1 amplitude modulation
        0x00, // op1 sustain level
        0x00, // op1 sustain rate
        0x00, // op1 release rate
        0x00, // op1 total level
        0x00, // op1 ssg eg
        // ...
    }; // 11 bytes

    Total bytes in FM definition: 49 bytes
 */

    mock_log_enable_checks();

    const u8 type = 0x00; // 0x00 = FM, 0x01 = PSG
    const u8 program = 0x05; // MIDI program 6

    FmChannel fmChannel = { 0 };
    memcpy(&fmChannel, &TEST_M_BANK_0_INST_0_GRANDPIANO, sizeof(FmChannel));
    fmChannel.algorithm = 0x07;

    u8 msg[10 + (MAX_FM_OPERATORS * 11)];
    msg[0] = SYSEX_MANU_EXTENDED;
    msg[1] = SYSEX_MANU_REGION;
    msg[2] = SYSEX_MANU_ID;
    msg[3] = SYSEX_COMMAND_STORE_PROGRAM;
    msg[4] = type;
    msg[5] = program;
    msg[6] = fmChannel.algorithm;
    msg[7] = fmChannel.feedback;
    msg[8] = fmChannel.ams;
    msg[9] = fmChannel.fms;
    for (u16 i = 0; i < MAX_FM_OPERATORS; i++) {
        msg[10 + i * 11] = fmChannel.operators[i].multiple;
        msg[11 + i * 11] = fmChannel.operators[i].detune;
        msg[12 + i * 11] = fmChannel.operators[i].attackRate;
        msg[13 + i * 11] = fmChannel.operators[i].rateScaling;
        msg[14 + i * 11] = fmChannel.operators[i].decayRate;
        msg[15 + i * 11] = fmChannel.operators[i].amplitudeModulation;
        msg[16 + i * 11] = fmChannel.operators[i].sustainLevel;
        msg[17 + i * 11] = fmChannel.operators[i].sustainRate;
        msg[18 + i * 11] = fmChannel.operators[i].releaseRate;
        msg[19 + i * 11] = fmChannel.operators[i].totalLevel;
        msg[20 + i * 11] = fmChannel.operators[i].ssgEg;
    }
    __real_midi_sysex(&msg[0], sizeof(msg));

    expect_synth_preset(FM_CH1, &fmChannel);
    __real_midi_program(MIDI_CHANNEL_1, program);
}

void test_midi_sysex_logs_warning_if_program_store_length_is_incorrect(UNUSED void** state)
{
    mock_log_enable_checks();

    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_STORE_PROGRAM, 0x00, 0x05 };

    expect_log_warn("Invalid store program data length: %d");
    __real_midi_sysex(sequence, sizeof(sequence));
}

void test_midi_sysex_logs_warning_if_program_store_type_is_incorrect(UNUSED void** state)
{
    mock_log_enable_checks();

    const u8 sequence[10 + (MAX_FM_OPERATORS * 11)] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION,
        SYSEX_MANU_ID, SYSEX_COMMAND_STORE_PROGRAM, 0x01, 0x05 };

    expect_log_warn("Invalid store program type: %d");
    __real_midi_sysex(sequence, sizeof(sequence));
}
