#include "test_midi.h"

int test_midi_setup(UNUSED void** state)
{
    const Channel M_BANK_0_INST_0_GRANDPIANO = { 2, 0, 3, 0, 0, 0, 0,
        { { 1, 0, 26, 1, 7, 0, 7, 4, 1, 39, 0 },
            { 4, 6, 24, 1, 9, 0, 6, 9, 7, 36, 0 },
            { 2, 7, 31, 3, 23, 0, 9, 15, 1, 4, 0 },
            { 1, 3, 27, 2, 4, 0, 10, 4, 6, 2, 0 } } };

    const Channel M_BANK_0_INST_1_BRIGHTPIANO = { 5, 7, 3, 0, 0, 0, 0,
        { { 4, 2, 27, 1, 9, 0, 11, 5, 6, 33, 0 },
            { 4, 5, 27, 1, 9, 0, 7, 9, 7, 18, 0 },
            { 1, 2, 27, 1, 5, 1, 10, 5, 6, 8, 0 },
            { 6, 5, 27, 1, 9, 0, 3, 8, 7, 9, 0 } } };

    const Channel P_BANK_0_INST_30_CASTANETS = { 4, 3, 3, 0, 0, 0, 0,
        { { 9, 0, 31, 0, 11, 0, 15, 0, 15, 23, 0 },
            { 1, 0, 31, 0, 19, 0, 15, 0, 15, 15, 0 },
            { 4, 0, 31, 2, 20, 0, 15, 0, 15, 13, 0 },
            { 2, 0, 31, 2, 20, 0, 15, 0, 15, 13, 0 } } };

    const Channel* const M_BANK_0[128]
        = { &M_BANK_0_INST_0_GRANDPIANO, &M_BANK_0_INST_1_BRIGHTPIANO };

    Channel const* P_BANK_0[128];
    P_BANK_0[30] = &P_BANK_0_INST_30_CASTANETS;

    expect_any(__wrap_synth_init, defaultPreset);
    midi_init((Channel**)M_BANK_0, (PercussionPreset**)P_BANK_0);
    // for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
    //     expect_synth_pitch_any();

    //     __real_midi_pitchBend(chan, 0);
    // }
    return 0;
}

void test_midi_polyphonic_mode_returns_state(UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 127);

    assert_true(__real_midi_getPolyphonic());

    __real_midi_cc(0, CC_POLYPHONIC_MODE, 0);
}

void test_midi_sets_all_notes_off(UNUSED void** state)
{
    expect_value(__wrap_synth_noteOff, channel, 0);

    __real_midi_cc(0, CC_ALL_NOTES_OFF, 0);
}

void test_midi_sets_all_sound_off(UNUSED void** state)
{
    expect_value(__wrap_synth_noteOff, channel, 0);

    __real_midi_cc(0, CC_ALL_SOUND_OFF, 0);
}

void test_midi_sets_unknown_CC(UNUSED void** state)
{
    u8 expectedController = 0x9;
    u8 expectedValue = 0x50;

    __real_midi_cc(0, expectedController, expectedValue);

    ControlChange* cc = midi_lastUnknownCC();

    assert_int_equal(cc->controller, expectedController);
    assert_int_equal(cc->value, expectedValue);
}

void test_midi_exposes_channel_mappings(UNUSED void** state)
{
    const u8 expectedMappings[MIDI_CHANNELS]
        = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 6, 7, 8, 6, 7, 8 };

    u8 mappings[MIDI_CHANNELS];
    __real_midi_mappings(mappings);

    assert_memory_equal(mappings, expectedMappings, sizeof(expectedMappings));
}
