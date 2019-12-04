#include "test_midi.h"

static void test_midi_sysex_sends_all_notes_off(UNUSED void** state)
{
    const u8 sysExGeneralMidiResetSequence[] = { 0x7E, 0x7F, 0x09, 0x01 };

    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        expect_any(__wrap_psg_frequency, channel);
        expect_any(__wrap_psg_frequency, freq);
        expect_any(__wrap_psg_attenuation, channel);
        expect_any(__wrap_psg_attenuation, attenuation);

        __real_midi_noteOn(chan, 60, 127);
    }

    expect_value(__wrap_synth_noteOff, channel, 0);
    expect_value(__wrap_synth_noteOff, channel, 1);
    expect_value(__wrap_synth_noteOff, channel, 2);
    expect_value(__wrap_synth_noteOff, channel, 3);
    expect_value(__wrap_synth_noteOff, channel, 4);
    expect_value(__wrap_synth_noteOff, channel, 5);
    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_value(__wrap_psg_attenuation, attenuation, PSG_ATTENUATION_SILENCE);
    expect_value(__wrap_psg_attenuation, channel, 1);
    expect_value(__wrap_psg_attenuation, attenuation, PSG_ATTENUATION_SILENCE);
    expect_value(__wrap_psg_attenuation, channel, 2);
    expect_value(__wrap_psg_attenuation, attenuation, PSG_ATTENUATION_SILENCE);
    expect_value(__wrap_psg_attenuation, channel, 3);
    expect_value(__wrap_psg_attenuation, attenuation, PSG_ATTENUATION_SILENCE);

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
    const u8 SYSEX_REMAP_MIDI_CHANNEL = 0x00;
    const u8 SYSEX_REMAP_DESTINATION_FIRST_PSG_CHANNEL = 0x06;

    const u8 sequence[] = { SYSEX_EXTENDED_MANU_ID_SECTION,
        SYSEX_UNUSED_EUROPEAN_SECTION, SYSEX_UNUSED_MANU_ID,
        SYSEX_REMAP_COMMAND_ID, SYSEX_REMAP_MIDI_CHANNEL,
        SYSEX_REMAP_DESTINATION_FIRST_PSG_CHANNEL };

    __real_midi_sysex(sequence, sizeof(sequence));

    expect_value(__wrap_psg_frequency, channel, 0);
    expect_any(__wrap_psg_frequency, freq);
    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_any(__wrap_psg_attenuation, attenuation);

    __real_midi_noteOn(0, 60, 127);
}

static void test_midi_sysex_remaps_midi_channel_to_fm(UNUSED void** state)
{
    const u8 SYSEX_REMAP_MIDI_CHANNEL = 0x00;
    const u8 SYSEX_REMAP_DESTINATION_SECOND_FM_CHANNEL = 0x01;

    const u8 sequence[] = { SYSEX_EXTENDED_MANU_ID_SECTION,
        SYSEX_UNUSED_EUROPEAN_SECTION, SYSEX_UNUSED_MANU_ID,
        SYSEX_REMAP_COMMAND_ID, SYSEX_REMAP_MIDI_CHANNEL,
        SYSEX_REMAP_DESTINATION_SECOND_FM_CHANNEL };

    __real_midi_sysex(sequence, sizeof(sequence));

    expect_value(__wrap_synth_pitch, channel, 1);
    expect_any(__wrap_synth_pitch, octave);
    expect_any(__wrap_synth_pitch, freqNumber);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 1);

    __real_midi_noteOn(0, 60, 127);
}

static void test_midi_sysex_unassigns_midi_channel(UNUSED void** state)
{
    const u8 SYSEX_REMAP_MIDI_CHANNEL = 0x00;
    const u8 SYSEX_REMAP_UNASSIGN_CHANNEL = 0x7F;

    const u8 sequence[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_REMAP_COMMAND_ID,
              SYSEX_REMAP_MIDI_CHANNEL, SYSEX_REMAP_UNASSIGN_CHANNEL };

    __real_midi_sysex(sequence, sizeof(sequence));

    __real_midi_noteOn(0, 60, 127);
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
    const u8 sequence[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_REMAP_COMMAND_ID };

    __real_midi_sysex(sequence, 4);
}

static void test_midi_sysex_enables_dynamic_channel_mode(UNUSED void** state)
{
    const u8 sequence[] = {
        SYSEX_EXTENDED_MANU_ID_SECTION,
        SYSEX_UNUSED_EUROPEAN_SECTION,
        SYSEX_UNUSED_MANU_ID,
        SYSEX_DYNAMIC_COMMAND_ID,
        SYSEX_DYNAMIC_ENABLED,
    };

    __real_midi_sysex(sequence, sizeof(sequence));

    print_message("Initial note");
    expect_synth_pitch(0, 4, 0x28d);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);
    __real_midi_noteOn(0, 60, 127);

    print_message("Second note");
    expect_synth_pitch(1, 4, 0x2b4);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 1);
    __real_midi_noteOn(0, 61, 127);
}

static void test_midi_sysex_disables_fm_parameter_CCs(UNUSED void** state)
{
    const u8 sequence[] = {
        SYSEX_EXTENDED_MANU_ID_SECTION,
        SYSEX_UNUSED_EUROPEAN_SECTION,
        SYSEX_UNUSED_MANU_ID,
        SYSEX_NON_GENERAL_MIDI_CC_COMMAND_ID,
        SYSEX_NON_GENERAL_MIDI_CC_DISABLED,
    };

    __real_midi_sysex(sequence, sizeof(sequence));

#define CC_GENMDM_FM_ALGORITHM 14
#define CC_GENMDM_FM_FEEDBACK 15
#define CC_GENMDM_TOTAL_LEVEL_OP1 16
#define CC_GENMDM_TOTAL_LEVEL_OP2 17
#define CC_GENMDM_TOTAL_LEVEL_OP3 18
#define CC_GENMDM_TOTAL_LEVEL_OP4 19
#define CC_GENMDM_MULTIPLE_OP1 20
#define CC_GENMDM_MULTIPLE_OP2 21
#define CC_GENMDM_MULTIPLE_OP3 22
#define CC_GENMDM_MULTIPLE_OP4 23
#define CC_GENMDM_DETUNE_OP1 24
#define CC_GENMDM_DETUNE_OP2 25
#define CC_GENMDM_DETUNE_OP3 26
#define CC_GENMDM_DETUNE_OP4 27
#define CC_GENMDM_RATE_SCALING_OP1 39
#define CC_GENMDM_RATE_SCALING_OP2 40
#define CC_GENMDM_RATE_SCALING_OP3 41
#define CC_GENMDM_RATE_SCALING_OP4 42
#define CC_GENMDM_ATTACK_RATE_OP1 43
#define CC_GENMDM_ATTACK_RATE_OP2 44
#define CC_GENMDM_ATTACK_RATE_OP3 45
#define CC_GENMDM_ATTACK_RATE_OP4 46
#define CC_GENMDM_FIRST_DECAY_RATE_OP1 47
#define CC_GENMDM_FIRST_DECAY_RATE_OP2 48
#define CC_GENMDM_FIRST_DECAY_RATE_OP3 49
#define CC_GENMDM_FIRST_DECAY_RATE_OP4 50
#define CC_GENMDM_SECOND_DECAY_RATE_OP1 51
#define CC_GENMDM_SECOND_DECAY_RATE_OP2 52
#define CC_GENMDM_SECOND_DECAY_RATE_OP3 53
#define CC_GENMDM_SECOND_DECAY_RATE_OP4 54
#define CC_GENMDM_SECOND_AMPLITUDE_OP1 55
#define CC_GENMDM_SECOND_AMPLITUDE_OP2 56
#define CC_GENMDM_SECOND_AMPLITUDE_OP3 57
#define CC_GENMDM_SECOND_AMPLITUDE_OP4 58
#define CC_GENMDM_RELEASE_RATE_OP1 59
#define CC_GENMDM_RELEASE_RATE_OP2 60
#define CC_GENMDM_RELEASE_RATE_OP3 61
#define CC_GENMDM_RELEASE_RATE_OP4 62
#define CC_GENMDM_AMPLITUDE_MODULATION_OP1 70
#define CC_GENMDM_AMPLITUDE_MODULATION_OP2 71
#define CC_GENMDM_AMPLITUDE_MODULATION_OP3 72
#define CC_GENMDM_AMPLITUDE_MODULATION_OP4 73
#define CC_GENMDM_GLOBAL_LFO_ENABLE 74
#define CC_GENMDM_FMS 75
#define CC_GENMDM_AMS 76
#define CC_GENMDM_STEREO 77
#define CC_POLYPHONIC_MODE 80
#define CC_GENMDM_SSG_EG_OP1 90
#define CC_GENMDM_SSG_EG_OP2 91
#define CC_GENMDM_SSG_EG_OP3 92
#define CC_GENMDM_SSG_EG_OP4 93
#define CC_GENMDM_GLOBAL_LFO_FREQUENCY 1

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
