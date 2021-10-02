#include "test_midi.h"

static void remapChannel(u8 midiChannel, u8 deviceChannel)
{
    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION,
        SYSEX_MANU_ID, SYSEX_COMMAND_REMAP, midiChannel, deviceChannel };

    __real_midi_sysex(sequence, sizeof(sequence));
}

static void test_midi_sysex_sends_all_notes_off(UNUSED void** state)
{
    const u8 sysExGeneralMidiResetSequence[] = { 0x7E, 0x7F, 0x09, 0x01 };

    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        expect_any_psg_tone();
        expect_any_psg_attenuation();
        __real_midi_note_on(chan, 60, 127);
    }

    expect_value(__wrap_synth_noteOff, channel, 0);
    expect_value(__wrap_synth_noteOff, channel, 1);
    expect_value(__wrap_synth_noteOff, channel, 2);
    expect_value(__wrap_synth_noteOff, channel, 3);
    expect_value(__wrap_synth_noteOff, channel, 4);
    expect_value(__wrap_synth_noteOff, channel, 5);

    expect_psg_attenuation(0, PSG_ATTENUATION_SILENCE);
    expect_psg_attenuation(1, PSG_ATTENUATION_SILENCE);
    expect_psg_attenuation(2, PSG_ATTENUATION_SILENCE);
    expect_psg_attenuation(3, PSG_ATTENUATION_SILENCE);

    __real_midi_sysex(
        sysExGeneralMidiResetSequence, sizeof(sysExGeneralMidiResetSequence));
}

static void test_midi_sysex_general_midi_reset_resets_synth_volume(
    UNUSED void** state)
{
    const u8 sysExGeneralMidiResetSequence[] = { 0x7E, 0x7F, 0x09, 0x01 };

    expect_value(__wrap_synth_volume, channel, 0);
    expect_value(__wrap_synth_volume, volume, 64);

    print_message("Setting volume to half-way\n");
    __real_midi_cc(0, CC_VOLUME, 64);

    expect_value(__wrap_synth_volume, channel, 0);
    expect_value(__wrap_synth_volume, volume, MAX_MIDI_VOLUME);
    expect_value(__wrap_synth_noteOff, channel, 0);
    expect_value(__wrap_synth_noteOff, channel, 1);
    expect_value(__wrap_synth_noteOff, channel, 2);
    expect_value(__wrap_synth_noteOff, channel, 3);
    expect_value(__wrap_synth_noteOff, channel, 4);
    expect_value(__wrap_synth_noteOff, channel, 5);

    print_message("Sending General MIDI reset\n");
    __real_midi_sysex(
        sysExGeneralMidiResetSequence, sizeof(sysExGeneralMidiResetSequence));
}

static void test_midi_sysex_ignores_unknown_sysex(UNUSED void** state)
{
    const u8 sysExGeneralMidiResetSequence[] = { 0x12 };

    __real_midi_sysex(
        sysExGeneralMidiResetSequence, sizeof(sysExGeneralMidiResetSequence));
}

static void test_midi_sysex_remaps_midi_channel_to_psg(UNUSED void** state)
{
    const u8 FM_CHAN_1 = 0;
    const u8 MIDI_CHAN_1 = 0;
    const u8 UNASSIGNED_MIDI = 0x7F;
    const u8 PSG_TONE_1 = 6;

    remapChannel(UNASSIGNED_MIDI, FM_CHAN_1);
    remapChannel(MIDI_CHAN_1, PSG_TONE_1);

    expect_any_psg_tone_on_channel(0);
    expect_psg_attenuation(0, PSG_ATTENUATION_LOUDEST);

    __real_midi_note_on(0, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);
}

static void test_midi_sysex_remaps_midi_channel_to_fm(UNUSED void** state)
{
    const u8 FM_CHAN_2 = 1;
    const u8 MIDI_CHAN_1 = 0;
    const u8 UNASSIGNED_MIDI = 0x7F;
    const u8 FM_CHAN_1 = 0;

    remapChannel(UNASSIGNED_MIDI, FM_CHAN_1);
    remapChannel(MIDI_CHAN_1, FM_CHAN_2);

    expect_value(__wrap_synth_pitch, channel, 1);
    expect_any(__wrap_synth_pitch, octave);
    expect_any(__wrap_synth_pitch, freqNumber);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 1);

    __real_midi_note_on(0, 60, MAX_MIDI_VOLUME);
}

static void test_midi_sysex_unassigns_midi_channel(UNUSED void** state)
{
    remapChannel(0, 0x7F);

    __real_midi_note_on(0, 60, MAX_MIDI_VOLUME);
}

static void test_midi_sysex_does_nothing_for_empty_payload(UNUSED void** state)
{
    const u16 length = 0;
    u8 seq[1];

    __real_midi_sysex(seq, length);
}

static void test_midi_sysex_handles_incomplete_channel_mapping_command(
    UNUSED void** state)
{
    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION,
        SYSEX_MANU_ID, SYSEX_COMMAND_REMAP };

    __real_midi_sysex(sequence, 4);
}

static void test_midi_sysex_enables_dynamic_channel_mode(UNUSED void** state)
{
    const u8 sequence[] = {
        SYSEX_MANU_EXTENDED,
        SYSEX_MANU_REGION,
        SYSEX_MANU_ID,
        SYSEX_COMMAND_DYNAMIC,
        SYSEX_DYNAMIC_ENABLED,
    };

    __real_midi_sysex(sequence, sizeof(sequence));

    print_message("Initial note");
    expect_synth_pitch(0, 4, 0x28d);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);
    __real_midi_note_on(0, 60, MAX_MIDI_VOLUME);

    print_message("Second note");
    expect_synth_pitch(1, 4, 0x2b4);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 1);
    __real_midi_note_on(0, 61, MAX_MIDI_VOLUME);
}

static void test_midi_sysex_disables_fm_parameter_CCs(UNUSED void** state)
{
    const u8 sequence[] = {
        SYSEX_MANU_EXTENDED,
        SYSEX_MANU_REGION,
        SYSEX_MANU_ID,
        SYSEX_COMMAND_NON_GENERAL_MIDI_CCS,
        SYSEX_NON_GENERAL_MIDI_CCS_DISABLED,
    };

    __real_midi_sysex(sequence, sizeof(sequence));

    const u8 nonGeneralMidiCCs[] = { CC_GENMDM_FM_ALGORITHM,
        CC_GENMDM_FM_FEEDBACK, CC_GENMDM_TOTAL_LEVEL_OP1, CC_GENMDM_DETUNE_OP4,
        CC_GENMDM_RATE_SCALING_OP1, CC_GENMDM_RELEASE_RATE_OP4,
        CC_GENMDM_AMPLITUDE_MODULATION_OP1, CC_GENMDM_STEREO,
        CC_GENMDM_SSG_EG_OP1, CC_GENMDM_SSG_EG_OP4,
        CC_GENMDM_GLOBAL_LFO_FREQUENCY };

    for (u16 i = 0; i < sizeof(nonGeneralMidiCCs) / sizeof(u8); i++) {
        u8 cc = nonGeneralMidiCCs[i];
        __real_midi_cc(0, cc, 1);
    }
}

static void test_midi_sysex_loads_psg_envelope(UNUSED void** state)
{
    wraps_enable_logging_checks();

    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION,
        SYSEX_MANU_ID, SYSEX_COMMAND_LOAD_PSG_ENVELOPE, 0x01, 0x01 };

    const u8 eef[] = { 0x11, EEF_END };
    const u8 eefLength = sizeof(eef) / sizeof(u8);

    expect_memory(__wrap_midi_psg_loadEnvelope, eef, eef, eefLength);

    expect_memory(__wrap_log_info, fmt, "Loaded User Defined Envelope", 29);

    __real_midi_sysex(sequence, sizeof(sequence));
}

static void test_midi_sysex_inverts_total_level_values(UNUSED void** state)
{
    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION,
        SYSEX_MANU_ID, SYSEX_COMMAND_INVERT_TOTAL_LEVEL, 0x01 };

    __real_midi_sysex(sequence, sizeof(sequence));

    expect_value(__wrap_synth_operatorTotalLevel, channel, 0);
    expect_value(__wrap_synth_operatorTotalLevel, op, 0);
    expect_value(__wrap_synth_operatorTotalLevel, totalLevel, 126);
    __real_midi_cc(0, CC_GENMDM_TOTAL_LEVEL_OP1, 1);
}

static void test_midi_sysex_sets_original_total_level_values(
    UNUSED void** state)
{
    const u8 invert_sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION,
        SYSEX_MANU_ID, SYSEX_COMMAND_INVERT_TOTAL_LEVEL, 0x01 };

    __real_midi_sysex(invert_sequence, sizeof(invert_sequence));
    expect_value(__wrap_synth_operatorTotalLevel, channel, 0);
    expect_value(__wrap_synth_operatorTotalLevel, op, 0);
    expect_value(__wrap_synth_operatorTotalLevel, totalLevel, 126);
    __real_midi_cc(0, CC_GENMDM_TOTAL_LEVEL_OP1, 1);

    const u8 original_sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION,
        SYSEX_MANU_ID, SYSEX_COMMAND_INVERT_TOTAL_LEVEL, 0x00 };

    __real_midi_sysex(original_sequence, sizeof(original_sequence));
    expect_value(__wrap_synth_operatorTotalLevel, channel, 0);
    expect_value(__wrap_synth_operatorTotalLevel, op, 0);
    expect_value(__wrap_synth_operatorTotalLevel, totalLevel, 126);
    __real_midi_cc(0, CC_GENMDM_TOTAL_LEVEL_OP1, 126);
}
