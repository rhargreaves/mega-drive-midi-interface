#include "test_midi.h"

static int test_midi_setup(UNUSED void** state)
{
    midi_init();
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch_any();

        __real_midi_pitchBend(chan, 0);
    }
    return 0;
}

static void test_midi_directs_channels_above_10_to_psg(UNUSED void** state)
{
    for (int chan = 11; chan < 16; chan++) {
        expect_any(__wrap_psg_frequency, channel);
        expect_any(__wrap_psg_frequency, freq);
        expect_any(__wrap_psg_attenuation, channel);
        expect_any(__wrap_psg_attenuation, attenuation);

        __real_midi_noteOn(chan, 60, 127);
    }
}

static void test_midi_polyphonic_mode_returns_state(UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 127);

    assert_true(__real_midi_getPolyphonic());

    __real_midi_cc(0, CC_POLYPHONIC_MODE, 0);
}

static void test_midi_sets_all_notes_off(UNUSED void** state)
{
    expect_value(__wrap_synth_noteOff, channel, 0);

    __real_midi_cc(0, CC_ALL_NOTES_OFF, 0);
}

static void test_midi_sets_unknown_CC(UNUSED void** state)
{
    u8 expectedController = 0x9;
    u8 expectedValue = 0x50;

    __real_midi_cc(0, expectedController, expectedValue);

    ControlChange* cc = midi_lastUnknownCC();

    assert_int_equal(cc->controller, expectedController);
    assert_int_equal(cc->value, expectedValue);
}
