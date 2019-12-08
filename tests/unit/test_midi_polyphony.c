#include "test_midi.h"

static void test_midi_polyphonic_mode_sends_CCs_to_all_FM_channels(
    UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 127);

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch_any();
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_noteOn(0, A_SHARP, MAX_MIDI_VOLUME);
    }
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_value(__wrap_synth_algorithm, channel, chan);
        expect_value(__wrap_synth_algorithm, algorithm, 1);
    }

    __real_midi_cc(0, CC_GENMDM_FM_ALGORITHM, 16);
}

static void test_midi_set_overflow_flag_on_polyphony_breach(UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 127);

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 6, 1164);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_noteOn(0, A_SHARP, 127);
    }
    for (int chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN - 1; chan++) {
        expect_value(__wrap_psg_frequency, channel, chan - MIN_PSG_CHAN);
        expect_any(__wrap_psg_frequency, freq);
        expect_value(__wrap_psg_attenuation, channel, chan - MIN_PSG_CHAN);
        expect_value(__wrap_psg_attenuation, attenuation, 0);

        __real_midi_noteOn(0, A_SHARP, 127);
    }

    __real_midi_noteOn(0, A_SHARP, 127);

    assert_true(midi_overflow());
}

static void test_midi_clears_overflow_flag(UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 127);

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_value(__wrap_synth_pitch, channel, chan);
        expect_any(__wrap_synth_pitch, octave);
        expect_any(__wrap_synth_pitch, freqNumber);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_noteOn(0, A_SHARP + chan, 127);
    }
    for (int chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN - 1; chan++) {
        expect_value(__wrap_psg_frequency, channel, chan - MIN_PSG_CHAN);
        expect_any(__wrap_psg_frequency, freq);
        expect_value(__wrap_psg_attenuation, channel, chan - MIN_PSG_CHAN);
        expect_value(__wrap_psg_attenuation, attenuation, 0);

        __real_midi_noteOn(0, A_SHARP + chan, 127);
    }

    __real_midi_noteOn(0, A_SHARP + MAX_PSG_CHAN, 127);

    assert_true(midi_overflow());

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_value(__wrap_synth_noteOff, channel, chan);

        __real_midi_noteOff(0, A_SHARP + chan);
    }
    for (int chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN - 1; chan++) {
        expect_any(__wrap_psg_attenuation, channel);
        expect_any(__wrap_psg_attenuation, attenuation);

        __real_midi_noteOff(0, A_SHARP + chan);
    }

    expect_value(__wrap_synth_pitch, channel, 0);
    expect_any(__wrap_synth_pitch, octave);
    expect_any(__wrap_synth_pitch, freqNumber);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);

    __real_midi_noteOn(0, A_SHARP, 127);

    assert_false(midi_overflow());
}

static void test_midi_polyphonic_mode_uses_multiple_fm_channels(
    UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 127);

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(0, 6, 1164);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, 0);

        __real_midi_noteOn(chan, A_SHARP, 127);

        expect_synth_pitch(1, 7, 0x269);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, 1);

        __real_midi_noteOn(chan, B, 127);

        expect_value(__wrap_synth_noteOff, channel, 0);

        __real_midi_noteOff(chan, A_SHARP);

        expect_value(__wrap_synth_noteOff, channel, 1);

        __real_midi_noteOff(chan, B);
    }

    __real_midi_cc(0, CC_POLYPHONIC_MODE, 0);
}

static void test_midi_polyphonic_mode_note_off_silences_all_matching_pitch(
    UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 127);

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(0, 6, 1164);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, 0);

        __real_midi_noteOn(chan, A_SHARP, 127);

        expect_synth_pitch(1, 6, 1164);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, 1);

        __real_midi_noteOn(chan, A_SHARP, 127);

        expect_value(__wrap_synth_noteOff, channel, 0);
        expect_value(__wrap_synth_noteOff, channel, 1);

        __real_midi_noteOff(chan, A_SHARP);
    }

    __real_midi_cc(0, CC_POLYPHONIC_MODE, 0);
}

static void test_midi_sets_all_notes_off_in_polyphonic_mode(UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 64);

    expect_value(__wrap_synth_pitch, channel, 0);
    expect_any(__wrap_synth_pitch, octave);
    expect_any(__wrap_synth_pitch, freqNumber);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);

    __real_midi_noteOn(0, A_SHARP, 127);

    expect_value(__wrap_synth_pitch, channel, 1);
    expect_any(__wrap_synth_pitch, octave);
    expect_any(__wrap_synth_pitch, freqNumber);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 1);

    __real_midi_noteOn(0, B, 127);

    expect_value(__wrap_synth_noteOff, channel, 0);
    expect_value(__wrap_synth_noteOff, channel, 1);
    expect_value(__wrap_synth_noteOff, channel, 2);
    expect_value(__wrap_synth_noteOff, channel, 3);
    expect_value(__wrap_synth_noteOff, channel, 4);
    expect_value(__wrap_synth_noteOff, channel, 5);

    __real_midi_cc(0, CC_ALL_NOTES_OFF, 0);

    __real_midi_cc(0, CC_POLYPHONIC_MODE, 0);
}

static void test_midi_sets_polyphonic_mode(UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 64);

    assert_true(__real_midi_dynamicMode());
}

static void test_midi_unsets_polyphonic_mode(UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 0);

    assert_false(__real_midi_dynamicMode());
}

static void test_midi_sets_all_channel_mappings_when_setting_polyphonic_mode(
    UNUSED void** state)
{
    __real_midi_cc(0, CC_POLYPHONIC_MODE, 64);

    DeviceChannel* mappings = __real_midi_dynamicModeMappings();
    for (u8 i = 0; i <= DEV_CHAN_MAX_FM; i++) {
        DeviceChannel mapping = mappings[i];
        assert_int_equal(mapping.midiChannel, 0);
    }
}
