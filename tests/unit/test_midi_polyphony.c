#include "test_midi.h"

static void test_midi_polyphonic_mode_sends_CCs_to_all_FM_channels(UNUSED void** state)
{
    __real_midi_cc(0, CC_GENMDM_POLYPHONIC_MODE, 127);

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch_any();
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_note_on(0, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);
    }
    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_value(__wrap_synth_algorithm, channel, chan);
        expect_value(__wrap_synth_algorithm, algorithm, 1);
    }

    __real_midi_cc(0, CC_GENMDM_FM_ALGORITHM, 16);
}

static void test_midi_set_overflow_flag_on_polyphony_breach(UNUSED void** state)
{
    wraps_enable_logging_checks();
    __real_midi_cc(0, CC_GENMDM_POLYPHONIC_MODE, 127);

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(chan, 6, SYNTH_NTSC_AS);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);
        __real_midi_note_on(chan, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);
    }
    for (int chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN - 1; chan++) {
        expect_psg_tone(chan - MIN_PSG_CHAN, 0x3b);
        expect_psg_attenuation(chan - MIN_PSG_CHAN, 0);
        __real_midi_note_on(chan, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);
    }

    expect_any(__wrap_log_warn, fmt);
    __real_midi_note_on(DEV_CHAN_MAX_PSG + 1, MIDI_PITCH_AS6, 127);
}

static void test_midi_polyphonic_mode_uses_multiple_fm_channels(UNUSED void** state)
{
    __real_midi_cc(0, CC_GENMDM_POLYPHONIC_MODE, 127);

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(0, 6, SYNTH_NTSC_AS);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, 0);

        __real_midi_note_on(chan, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);

        expect_synth_pitch(1, 7, 0x25f);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, 1);

        __real_midi_note_on(chan, MIDI_PITCH_B6, MAX_MIDI_VOLUME);

        expect_value(__wrap_synth_noteOff, channel, 0);

        __real_midi_note_off(chan, MIDI_PITCH_AS6);

        expect_value(__wrap_synth_noteOff, channel, 1);

        __real_midi_note_off(chan, MIDI_PITCH_B6);
    }

    __real_midi_cc(0, CC_GENMDM_POLYPHONIC_MODE, 0);
}

static void test_midi_polyphonic_mode_note_off_silences_all_matching_pitch(UNUSED void** state)
{
    __real_midi_cc(0, CC_GENMDM_POLYPHONIC_MODE, 127);

    for (int chan = 0; chan <= MAX_FM_CHAN; chan++) {
        expect_synth_pitch(0, 6, SYNTH_NTSC_AS);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, 0);

        __real_midi_note_on(chan, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);

        expect_synth_pitch(1, 6, SYNTH_NTSC_AS);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, 1);

        __real_midi_note_on(chan, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);

        expect_value(__wrap_synth_noteOff, channel, 0);
        expect_value(__wrap_synth_noteOff, channel, 1);

        __real_midi_note_off(chan, MIDI_PITCH_AS6);
    }

    __real_midi_cc(0, CC_GENMDM_POLYPHONIC_MODE, 0);
}

static void test_midi_sets_all_notes_off_in_polyphonic_mode(UNUSED void** state)
{
    __real_midi_cc(0, CC_GENMDM_POLYPHONIC_MODE, 64);

    expect_value(__wrap_synth_pitch, channel, 0);
    expect_any(__wrap_synth_pitch, octave);
    expect_any(__wrap_synth_pitch, freqNumber);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);

    __real_midi_note_on(0, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);

    expect_value(__wrap_synth_pitch, channel, 1);
    expect_any(__wrap_synth_pitch, octave);
    expect_any(__wrap_synth_pitch, freqNumber);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 1);

    __real_midi_note_on(0, MIDI_PITCH_B6, MAX_MIDI_VOLUME);

    expect_value(__wrap_synth_noteOff, channel, 0);
    expect_value(__wrap_synth_noteOff, channel, 1);
    expect_value(__wrap_synth_noteOff, channel, 2);
    expect_value(__wrap_synth_noteOff, channel, 3);
    expect_value(__wrap_synth_noteOff, channel, 4);
    expect_value(__wrap_synth_noteOff, channel, 5);

    __real_midi_cc(0, CC_ALL_NOTES_OFF, 0);

    __real_midi_cc(0, CC_GENMDM_POLYPHONIC_MODE, 0);
}

static void test_midi_sets_polyphonic_mode(UNUSED void** state)
{
    __real_midi_cc(0, CC_GENMDM_POLYPHONIC_MODE, 64);

    assert_true(__real_midi_dynamic_mode());
}

static void test_midi_unsets_polyphonic_mode(UNUSED void** state)
{
    __real_midi_cc(0, CC_GENMDM_POLYPHONIC_MODE, 0);

    assert_false(__real_midi_dynamic_mode());
}

static void test_midi_sets_all_channel_mappings_when_setting_polyphonic_mode(UNUSED void** state)
{
    __real_midi_cc(0, CC_GENMDM_POLYPHONIC_MODE, 64);

    DeviceChannel* mappings = __real_midi_channel_mappings();
    for (u8 i = 0; i <= DEV_CHAN_MAX_FM; i++) {
        DeviceChannel mapping = mappings[i];
        assert_int_equal(mapping.midiChannel, 0);
    }
}
