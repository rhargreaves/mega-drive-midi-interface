#include "test_midi.h"

static int test_dynamic_midi_setup(UNUSED void** state)
{
    test_midi_setup(state);

    const u8 sequence[] = {
        SYSEX_EXTENDED_MANU_ID_SECTION,
        SYSEX_UNUSED_EUROPEAN_SECTION,
        SYSEX_UNUSED_MANU_ID,
        SYSEX_DYNAMIC_COMMAND_ID,
        SYSEX_DYNAMIC_ENABLED,
    };

    __real_midi_sysex(sequence, sizeof(sequence));

    return 0;
}

static void test_midi_dynamic_uses_all_channels(UNUSED void** state)
{
    const u8 octave = 4;
    const u16 freq = 0x28d;
    const u16 pitch = 60;

    print_message("FM channels...\n");
    for (u16 i = DEV_CHAN_MIN_FM; i <= DEV_CHAN_MAX_FM; i++) {
        expect_synth_pitch(i, octave, freq);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, i);

        __real_midi_noteOn(0, pitch, 127);
    }

    print_message("PSG channels (except noise)..\n");
    for (u16 i = DEV_CHAN_MIN_PSG; i <= DEV_CHAN_MAX_PSG - 1; i++) {
        u8 psgChan = i - DEV_CHAN_MIN_PSG;
        expect_value(__wrap_psg_frequency, channel, psgChan);
        expect_any(__wrap_psg_frequency, freq);
        expect_value(__wrap_psg_attenuation, channel, psgChan);
        expect_value(__wrap_psg_attenuation, attenuation, 0);

        __real_midi_noteOn(0, pitch, 127);
    }

    __real_midi_noteOn(0, pitch, 127);
}

static void test_midi_dynamic_tries_to_reuse_original_midi_channel_if_available(
    UNUSED void** state)
{
    const u8 octave = 4;
    const u16 freq = 0x28d;
    const u16 pitch = 60;

    for (u8 chan = 0; chan < 3; chan++) {
        expect_synth_pitch(chan, octave, freq);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_noteOn(chan, pitch, 127);
    }

    const u8 REUSE_MIDI_CHANNEL = 2;

    expect_value(__wrap_synth_noteOff, channel, REUSE_MIDI_CHANNEL);
    __real_midi_noteOff(REUSE_MIDI_CHANNEL, pitch);

    expect_synth_pitch(REUSE_MIDI_CHANNEL, octave, freq);
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, REUSE_MIDI_CHANNEL);

    __real_midi_noteOn(REUSE_MIDI_CHANNEL, pitch, 127);
}

static void test_midi_reports_dynamic_mode_enabled(UNUSED void** state)
{
    assert_true(__real_midi_dynamicMode);
}

static void test_midi_reports_dynamic_mode_disabled(UNUSED void** state)
{
    const u8 sequence[] = {
        SYSEX_EXTENDED_MANU_ID_SECTION,
        SYSEX_UNUSED_EUROPEAN_SECTION,
        SYSEX_UNUSED_MANU_ID,
        SYSEX_DYNAMIC_COMMAND_ID,
        SYSEX_DYNAMIC_DISABLED,
    };

    __real_midi_sysex(sequence, sizeof(sequence));

    assert_false(__real_midi_dynamicMode());
}

static void test_midi_exposes_dynamic_mode_mappings(UNUSED void** state)
{
    DeviceChannel* mappings = __real_midi_dynamicModeMappings();
    for (u8 i = DEV_CHAN_MIN_FM; i < DEV_CHAN_MAX_FM; i++) {
        DeviceChannel* mapping = &mappings[i];

        print_message("Chan %d\n", i);
        assert_false(mapping->noteOn);
        assert_int_equal(mapping->number, i);
    }
}

static void test_midi_dynamic_enables_percussive_mode_if_needed(
    UNUSED void** state)
{
    const u8 MIDI_KEY = 30;

    expect_value(__wrap_synth_preset, channel, 0);
    expect_any(__wrap_synth_preset, preset);

    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);

    print_message("Playing first drum\n");
    __real_midi_noteOn(GENERAL_MIDI_PERCUSSION_CHANNEL, MIDI_KEY, 127);

    expect_value(__wrap_synth_preset, channel, 1);
    expect_any(__wrap_synth_preset, preset);

    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 1);

    print_message("Playing second drum\n");
    __real_midi_noteOn(GENERAL_MIDI_PERCUSSION_CHANNEL, MIDI_KEY, 127);
}

static void test_midi_sets_presets_on_dynamic_channels(UNUSED void** state)
{
    expect_value(__wrap_synth_preset, channel, 0);
    expect_any(__wrap_synth_preset, preset);
    expect_any(__wrap_synth_stereo, channel);
    expect_any(__wrap_synth_stereo, mode);
    __real_midi_program(0, 2);

    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);

    print_message("Playing first note\n");
    __real_midi_noteOn(0, A_SHARP, 127);

    expect_value(__wrap_synth_preset, channel, 1);
    expect_any(__wrap_synth_preset, preset);
    expect_any(__wrap_synth_stereo, channel);
    expect_any(__wrap_synth_stereo, mode);
    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 1);

    print_message("Playing second note\n");
    __real_midi_noteOn(0, A_SHARP, 127);
}

static void test_midi_dynamic_does_not_send_percussion_to_psg_channels(
    UNUSED void** state)
{
    const u8 MIDI_KEY_IN_PSG_RANGE = A_SHARP;

    for (u8 chan = DEV_CHAN_MIN_FM; chan <= DEV_CHAN_MAX_FM; chan++) {
        expect_synth_pitch_any();
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);

        print_message("Playing FM note %d\n", chan);
        __real_midi_noteOn(0, MIDI_KEY_IN_PSG_RANGE, 127);
    }

    print_message("Drum should be dropped.\n");
    __real_midi_noteOn(
        GENERAL_MIDI_PERCUSSION_CHANNEL, MIDI_KEY_IN_PSG_RANGE, 127);
}

static void test_midi_sysex_resets_dynamic_mode_state(UNUSED void** state)
{
    const u8 sysExGeneralMidiResetSequence[] = { 0x7E, 0x7F, 0x09, 0x01 };
    const u8 octave = 4;
    const u16 freq = 0x28d;
    const u16 pitch = 60;

    for (u16 i = DEV_CHAN_MIN_FM; i <= DEV_CHAN_MAX_FM; i++) {
        expect_synth_pitch(i, octave, freq);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, i);

        __real_midi_noteOn(0, pitch, 127);
    }

    expect_value(__wrap_synth_noteOff, channel, 0);
    expect_value(__wrap_synth_noteOff, channel, 1);
    expect_value(__wrap_synth_noteOff, channel, 2);
    expect_value(__wrap_synth_noteOff, channel, 3);
    expect_value(__wrap_synth_noteOff, channel, 4);
    expect_value(__wrap_synth_noteOff, channel, 5);

    __real_midi_sysex(
        sysExGeneralMidiResetSequence, sizeof(sysExGeneralMidiResetSequence));

    DeviceChannel* mappings = __real_midi_dynamicModeMappings();
    for (u8 i = 0; i < DEV_CHANS; i++) {
        DeviceChannel* mapping = &mappings[i];
        assert_int_equal(mapping->midiChannel, DEFAULT_MIDI_CHANNEL);
        assert_int_equal(mapping->program, 0);
        assert_false(mapping->noteOn);
    }
}

static void test_midi_dynamic_sends_note_off_to_channel_playing_same_pitch(
    UNUSED void** state)
{
    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);

    __real_midi_noteOn(0, A_SHARP, 127);

    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 1);

    __real_midi_noteOn(0, B, 127);

    expect_value(__wrap_synth_noteOff, channel, 1);

    __real_midi_noteOff(0, B);
}

static void test_midi_dynamic_limits_percussion_notes(UNUSED void** state)
{
    const u8 DRUM_KEY = 30;

    for (u8 i = 0; i < 2; i++) {
        expect_synth_pitch_any();
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, i);
        expect_value(__wrap_synth_preset, channel, i);
        expect_any(__wrap_synth_preset, preset);

        print_message("Drum %d\n", i + 1);
        __real_midi_noteOn(GENERAL_MIDI_PERCUSSION_CHANNEL, DRUM_KEY, 127);
    }

    print_message("Drum 3 (dropped)\n");
    __real_midi_noteOn(GENERAL_MIDI_PERCUSSION_CHANNEL, DRUM_KEY, 127);
}

static void test_midi_dynamic_maintains_volume_on_remapping(UNUSED void** state)
{
    const u8 midi_vol = 50;
    const u8 expected_synth_vol = 0x32;

    expect_synth_volume(0, expected_synth_vol);
    print_message("Setting volume\n");
    __real_midi_cc(0, CC_VOLUME, midi_vol);

    expect_synth_pitch_any();
    expect_synth_volume(0, expected_synth_vol);
    expect_value(__wrap_synth_noteOn, channel, 0);

    print_message("Note 1\n");
    __real_midi_noteOn(0, A_SHARP, 127);

    expect_synth_pitch_any();
    expect_synth_volume(1, expected_synth_vol);
    expect_synth_volume(1, expected_synth_vol);
    expect_value(__wrap_synth_noteOn, channel, 1);

    print_message("Note 2\n");
    __real_midi_noteOn(0, B, 127);
}

static void test_midi_dynamic_sets_volume_on_playing_notes(UNUSED void** state)
{
    const u8 midi_vol_initial = 50;
    const u8 midi_vol_next = 75;
    const u8 expected_synth_vol_initial = 0x32;
    const u8 expected_synth_vol_next = 0x4b;

    expect_synth_volume(0, expected_synth_vol_initial);
    print_message("Setting volume\n");
    __real_midi_cc(0, CC_VOLUME, midi_vol_initial);

    expect_synth_pitch_any();
    expect_synth_volume(0, expected_synth_vol_initial);
    expect_value(__wrap_synth_noteOn, channel, 0);

    print_message("Note 1\n");
    __real_midi_noteOn(0, A_SHARP, 127);

    expect_synth_pitch_any();
    expect_synth_volume(1, expected_synth_vol_initial);
    expect_synth_volume(1, expected_synth_vol_initial);
    expect_value(__wrap_synth_noteOn, channel, 1);

    print_message("Note 2\n");
    __real_midi_noteOn(0, B, 127);

    expect_synth_volume(0, expected_synth_vol_next);
    expect_synth_volume(1, expected_synth_vol_next);
    print_message("Setting volume again\n");
    __real_midi_cc(0, CC_VOLUME, midi_vol_next);
}

static void test_midi_dynamic_maintains_pan_on_remapping(UNUSED void** state)
{
    const u8 midi_pan = 127;
    const u8 expected_synth_stereo = STEREO_MODE_RIGHT;

    expect_value(__wrap_synth_stereo, channel, 0);
    expect_value(__wrap_synth_stereo, mode, expected_synth_stereo);
    print_message("Setting pan\n");
    __real_midi_cc(0, CC_PAN, midi_pan);

    expect_synth_pitch_any();
    expect_synth_volume(0, 127);
    expect_value(__wrap_synth_noteOn, channel, 0);

    print_message("Note 1\n");
    __real_midi_noteOn(0, A_SHARP, 127);

    expect_synth_pitch_any();
    expect_synth_volume(1, 127);
    expect_value(__wrap_synth_stereo, channel, 1);
    expect_value(__wrap_synth_stereo, mode, expected_synth_stereo);
    expect_value(__wrap_synth_noteOn, channel, 1);

    print_message("Note 2\n");
    __real_midi_noteOn(0, B, 127);
}

static void test_midi_dynamic_maintains_pitch_bend_on_remapping(
    UNUSED void** state)
{
    /* Buggy behaviour characterisation */

    const u16 midi_bend = 0x3000;

    expect_synth_pitch(0, 6, 0x48c);
    expect_synth_volume(0, MAX_MIDI_VOLUME);
    expect_value(__wrap_synth_noteOn, channel, 0);

    print_message("Note 1\n");
    __real_midi_noteOn(0, A_SHARP, 127);

    print_message("Setting bend\n");
    expect_synth_pitch(0, 6, 0x4c2);
    __real_midi_pitchBend(0, midi_bend);

    expect_synth_pitch(1, 7, 0x29f);
    expect_synth_pitch(1, 7,
        0x269); // defaults back as pitch bend not taken into consideration on
                // note on
    expect_synth_volume(1, MAX_MIDI_VOLUME);
    expect_value(__wrap_synth_noteOn, channel, 1);

    print_message("Note 2\n");
    __real_midi_noteOn(0, B, 127);
}

static void test_midi_dynamic_resets_mappings_on_cc_121(UNUSED void** state)
{
    const u8 midiChannel = 2;

    for (u16 i = DEV_CHAN_MIN_FM; i <= DEV_CHAN_MAX_FM; i++) {
        expect_synth_pitch_any();
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, i);

        __real_midi_noteOn(midiChannel, A_SHARP, 127);
    }

    __real_midi_cc(midiChannel, 121, 0);

    DeviceChannel* channels = __real_midi_dynamicModeMappings();
    for (u16 i = DEV_CHAN_MIN_FM; i <= DEV_CHAN_MAX_FM; i++) {
        DeviceChannel* chan = &channels[i];
        assert_int_equal(chan->midiChannel, DEFAULT_MIDI_CHANNEL);
    }
}

static void test_midi_dynamic_all_notes_off_on_cc_123(UNUSED void** state)
{
    const u8 midiChannel = 2;

    for (u16 i = DEV_CHAN_MIN_FM; i <= DEV_CHAN_MAX_FM; i++) {
        expect_synth_pitch_any();
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, i);

        __real_midi_noteOn(midiChannel, A_SHARP, 127);
    }

    for (u16 i = DEV_CHAN_MIN_FM; i <= DEV_CHAN_MAX_FM; i++) {
        expect_value(__wrap_synth_noteOff, channel, i);
    }

    __real_midi_cc(midiChannel, 123, 0);

    DeviceChannel* channels = __real_midi_dynamicModeMappings();
    for (u16 i = DEV_CHAN_MIN_FM; i <= DEV_CHAN_MAX_FM; i++) {
        DeviceChannel* chan = &channels[i];
        assert_false(chan->noteOn);
    }
}

static void test_midi_dynamic_sysex_remaps_midi_channel(UNUSED void** state)
{
    const u8 MIDI_CHANNEL = 0x01;
    const u8 DESTINATION_FIRST_PSG_CHANNEL = 0x06;

    const u8 sequence[] = { SYSEX_EXTENDED_MANU_ID_SECTION,
        SYSEX_UNUSED_EUROPEAN_SECTION, SYSEX_UNUSED_MANU_ID,
        SYSEX_REMAP_COMMAND_ID, MIDI_CHANNEL, DESTINATION_FIRST_PSG_CHANNEL };

    __real_midi_sysex(sequence, sizeof(sequence));

    expect_value(__wrap_psg_frequency, channel, 0);
    expect_any(__wrap_psg_frequency, freq);
    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_any(__wrap_psg_attenuation, attenuation);

    __real_midi_noteOn(MIDI_CHANNEL, 60, 127);
}

static void test_midi_dynamic_sysex_removes_mapping_of_midi_channel(
    UNUSED void** state)
{
    const u8 MIDI_CHANNEL = 0x00;
    const u8 MIDI_CHANNEL_NO_MAPPING = 0x7F;
    const u8 DESTINATION_FIRST_PSG_CHANNEL = 0x06;

    const u8 sequence[] = { SYSEX_EXTENDED_MANU_ID_SECTION,
        SYSEX_UNUSED_EUROPEAN_SECTION, SYSEX_UNUSED_MANU_ID,
        SYSEX_REMAP_COMMAND_ID, MIDI_CHANNEL, DESTINATION_FIRST_PSG_CHANNEL };

    print_message("Assigning PSG to MIDI Chan 0\n");
    __real_midi_sysex(sequence, sizeof(sequence));

    expect_value(__wrap_psg_frequency, channel, 0);
    expect_any(__wrap_psg_frequency, freq);
    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_any(__wrap_psg_attenuation, attenuation);

    print_message("Playing note\n");
    __real_midi_noteOn(MIDI_CHANNEL, 60, 127);

    const u8 removeMappingSeq[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_REMAP_COMMAND_ID,
              MIDI_CHANNEL_NO_MAPPING, DESTINATION_FIRST_PSG_CHANNEL };

    print_message("Removing MIDI Chan 0 assignment\n");
    __real_midi_sysex(removeMappingSeq, sizeof(removeMappingSeq));

    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);

    print_message("Playing note\n");
    __real_midi_noteOn(MIDI_CHANNEL, 60, 127);
}
