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

    print_message("PSG channels...\n");
    for (u16 i = DEV_CHAN_MIN_PSG; i <= DEV_CHAN_MAX_PSG; i++) {
        u8 psgChan = i - DEV_CHAN_MIN_PSG;
        expect_value(__wrap_psg_frequency, channel, psgChan);
        expect_any(__wrap_psg_frequency, freq);
        expect_value(__wrap_psg_attenuation, channel, psgChan);
        expect_value(__wrap_psg_attenuation, attenuation, 0);

        __real_midi_noteOn(0, pitch, 127);
    }
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
