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

static void test_midi_dynamic_tries_to_use_original_midi_channel_if_available(
    UNUSED void** state)
{
    const u8 octave = 4;
    const u16 freq = 0x28d;
    const u16 pitch = 60;

    const u8 chans[3] = { 0, 2, 5 };

    for (u16 i = 0; i < 3; i++) {
        u8 chan = chans[i];
        expect_synth_pitch(chan, octave, freq);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);

        __real_midi_noteOn(chan, pitch, 127);
    }
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
    ChannelState* mappings = __real_midi_dynamicModeMappings();
    for (u8 i = DEV_CHAN_MIN_FM; i < DEV_CHAN_MAX_FM; i++) {
        ChannelState* mapping = &mappings[i];

        print_message("Chan %d\n", i);
        assert_false(mapping->noteOn);
        assert_int_equal(mapping->deviceChannel, i);
    }
}

static void test_midi_dynamic_enables_percussive_mode_if_needed(
    UNUSED void** state)
{
    const u8 MIDI_KEY = 30;

    midi_remapChannel(GENERAL_MIDI_PERCUSSION_CHANNEL, 5);

    expect_value(__wrap_synth_preset, channel, 5);
    expect_any(__wrap_synth_preset, preset);

    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 5);

    print_message("Playing first drum\n");
    __real_midi_noteOn(GENERAL_MIDI_PERCUSSION_CHANNEL, MIDI_KEY, 127);

    expect_value(__wrap_synth_preset, channel, 0);
    expect_any(__wrap_synth_preset, preset);

    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_value(__wrap_synth_noteOn, channel, 0);

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

    expect_value(__wrap_synth_preset, channel, 0);
    expect_any(__wrap_synth_preset, preset);
    expect_any(__wrap_synth_stereo, channel);
    expect_any(__wrap_synth_stereo, mode);
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
    const u8 MIDI_KEY = 30;
    const u8 MIDI_KEY_IN_PSG_RANGE = A_SHARP;

    midi_remapChannel(GENERAL_MIDI_PERCUSSION_CHANNEL, 0);

    for (u8 chan = DEV_CHAN_MIN_FM; chan <= DEV_CHAN_MAX_FM; chan++) {
        expect_value(__wrap_synth_preset, channel, chan);
        expect_any(__wrap_synth_preset, preset);
        expect_synth_pitch_any();
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, chan);

        print_message("Playing drum %d\n", chan);
        __real_midi_noteOn(GENERAL_MIDI_PERCUSSION_CHANNEL, MIDI_KEY, 127);
    }

    print_message("Drum %d should be dropped.\n", 6);
    __real_midi_noteOn(
        GENERAL_MIDI_PERCUSSION_CHANNEL, MIDI_KEY_IN_PSG_RANGE, 127);
}
