#include "test_midi_dynamic.h"
#include "test_midi.h"
#include "mocks/mock_psg.h"

#define LENGTH_OF(x) (sizeof(x) / sizeof((x)[0]))

static void setStickToDeviceType(bool enable)
{
    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_STICK_TO_DEVICE_TYPE, enable ? 1 : 0 };

    __real_midi_sysex(sequence, sizeof(sequence));
}

int test_dynamic_midi_setup(UNUSED void** state)
{
    test_midi_setup(state);

    const u8 sequence[] = {
        SYSEX_MANU_EXTENDED,
        SYSEX_MANU_REGION,
        SYSEX_MANU_ID,
        SYSEX_COMMAND_DYNAMIC,
        SYSEX_DYNAMIC_ENABLED,
    };

    __real_midi_sysex(sequence, sizeof(sequence));

    return 0;
}

void test_midi_dynamic_uses_all_channels(UNUSED void** state)
{
    const u8 octave = 4;
    const u16 freq = 0x284;
    const u16 pitch = 60;

    // FM channels
    for (u16 i = DEV_CHAN_MIN_FM; i <= DEV_CHAN_MAX_FM; i++) {
        expect_synth_pitch(i, octave, freq);
        expect_synth_volume_any();
        expect_value(__wrap_synth_note_on, channel, i);

        __real_midi_note_on(0, pitch, MAX_MIDI_VOLUME);
    }

    // PSG channels (except noise)
    for (u16 i = DEV_CHAN_MIN_PSG; i <= DEV_CHAN_MAX_PSG - 1; i++) {
        u8 psgChan = i - DEV_CHAN_MIN_PSG;
        expect_any_psg_tone_on_channel(psgChan);
        expect_psg_attenuation(psgChan, 0);
        __real_midi_note_on(0, pitch, MAX_MIDI_VOLUME);
    }
}

void test_midi_routing_switches_to_dynamic_on_gm_reset(UNUSED void** state)
{
    const u8 sysExGeneralMidiResetSequence[] = { 0x7E, 0x7F, 0x09, 0x01 };
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

void test_midi_dynamic_tries_to_reuse_original_midi_channel_if_available(UNUSED void** state)
{
    const u8 octave = 4;
    const u16 freq = 0x284;
    const u16 pitch = 60;

    for (u8 chan = 0; chan < 3; chan++) {
        expect_synth_pitch(chan, octave, freq);
        expect_synth_volume_any();
        expect_value(__wrap_synth_note_on, channel, chan);

        __real_midi_note_on(chan, pitch, MAX_MIDI_VOLUME);
    }

    const u8 REUSE_MIDI_CHANNEL = 2;

    expect_value(__wrap_synth_note_off, channel, REUSE_MIDI_CHANNEL);
    __real_midi_note_off(REUSE_MIDI_CHANNEL, pitch);

    expect_synth_pitch(REUSE_MIDI_CHANNEL, octave, freq);
    expect_synth_volume_any();
    expect_value(__wrap_synth_note_on, channel, REUSE_MIDI_CHANNEL);

    __real_midi_note_on(REUSE_MIDI_CHANNEL, pitch, MAX_MIDI_VOLUME);
}

void test_midi_dynamic_reuses_mapped_midi_channel_even_if_busy_if_sticking_to_device_type(
    UNUSED void** state)
{
    setStickToDeviceType(true);

    const u16 pitch = MIDI_PITCH_B6;
    const u8 REUSE_MIDI_CHANNEL = 2;

    for (u8 chan = DEV_CHAN_MIN_FM; chan <= DEV_CHAN_MAX_FM; chan++) {
        expect_synth_pitch_any();
        expect_synth_volume_any();
        expect_value(__wrap_synth_note_on, channel, chan);

        __real_midi_note_on(REUSE_MIDI_CHANNEL, pitch, MAX_MIDI_VOLUME);
    }

    expect_synth_pitch(0, 6, 0x47a);
    expect_synth_volume_any();
    expect_value(__wrap_synth_note_on, channel, 0);
    __real_midi_note_on(REUSE_MIDI_CHANNEL, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);
}

void test_midi_reports_dynamic_mode_enabled(UNUSED void** state)
{
    assert_true(__real_midi_dynamic_mode);
}

void test_midi_reports_dynamic_mode_disabled(UNUSED void** state)
{
    const u8 sequence[] = {
        SYSEX_MANU_EXTENDED,
        SYSEX_MANU_REGION,
        SYSEX_MANU_ID,
        SYSEX_COMMAND_DYNAMIC,
        SYSEX_DYNAMIC_DISABLED,
    };

    __real_midi_sysex(sequence, sizeof(sequence));

    assert_false(__real_midi_dynamic_mode());
}

void test_midi_exposes_dynamic_mode_mappings(UNUSED void** state)
{
    DeviceChannel* mappings = __real_midi_channel_mappings();
    for (u8 i = DEV_CHAN_MIN_FM; i < DEV_CHAN_MAX_FM; i++) {
        DeviceChannel* mapping = &mappings[i];

        assert_false(mapping->noteOn);
        assert_int_equal(mapping->num, i);
    }
}

void test_midi_dynamic_enables_percussive_mode_if_needed(UNUSED void** state)
{
    const u8 MIDI_KEY = 30;

    // Playing first drum
    expect_synth_preset(0, &TEST_P_BANK_0_INST_30_CASTANETS.channel);
    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_synth_note_on(0);
    __real_midi_note_on(GENERAL_MIDI_PERCUSSION_CHANNEL, MIDI_KEY, MAX_MIDI_VOLUME);

    // Playing second drum
    expect_synth_preset(1, &TEST_P_BANK_0_INST_30_CASTANETS.channel);
    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_synth_note_on(1);
    __real_midi_note_on(GENERAL_MIDI_PERCUSSION_CHANNEL, MIDI_KEY, MAX_MIDI_VOLUME);
}

void test_midi_dynamic_reapplies_program_on_non_percussion_channel(UNUSED void** state)
{
    const u8 MIDI_KEY = 30;

    // Play and stop drum
    expect_synth_preset(0, &TEST_P_BANK_0_INST_30_CASTANETS.channel);
    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_synth_note_on(0);
    __real_midi_note_on(GENERAL_MIDI_PERCUSSION_CHANNEL, MIDI_KEY, MAX_MIDI_VOLUME);
    expect_synth_note_off(0);
    __real_midi_note_off(GENERAL_MIDI_PERCUSSION_CHANNEL, MIDI_KEY);

    // Play non-percussion note
    expect_synth_preset(0, &TEST_M_BANK_0_INST_0_GRANDPIANO);
    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_synth_note_on(0);
    __real_midi_note_on(MIDI_CHANNEL_1, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
}

void test_midi_sets_presets_on_dynamic_channels(UNUSED void** state)
{
    expect_value(__wrap_synth_preset, channel, 0);
    expect_any(__wrap_synth_preset, preset);
    __real_midi_program(0, 2);

    // Playing first note
    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_value(__wrap_synth_note_on, channel, 0);
    __real_midi_note_on(0, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);

    // Playing second note
    expect_value(__wrap_synth_preset, channel, 1);
    expect_any(__wrap_synth_preset, preset);
    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_value(__wrap_synth_note_on, channel, 1);
    __real_midi_note_on(0, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);
}

void test_midi_dynamic_does_not_send_percussion_to_psg_channels(UNUSED void** state)
{
    const u8 MIDI_KEY_IN_PSG_RANGE = MIDI_PITCH_AS6;

    for (u8 chan = DEV_CHAN_MIN_FM; chan <= DEV_CHAN_MAX_FM; chan++) {
        // Playing FM note
        expect_synth_pitch_any();
        expect_synth_volume_any();
        expect_value(__wrap_synth_note_on, channel, chan);
        __real_midi_note_on(0, MIDI_KEY_IN_PSG_RANGE, MAX_MIDI_VOLUME);
    }

    // Drum should be dropped
    __real_midi_note_on(GENERAL_MIDI_PERCUSSION_CHANNEL, MIDI_KEY_IN_PSG_RANGE, MAX_MIDI_VOLUME);
}

void test_midi_sysex_resets_dynamic_mode_state(UNUSED void** state)
{
    const u8 sysExGeneralMidiResetSequence[] = { 0x7E, 0x7F, 0x09, 0x01 };
    const u8 octave = 4;
    const u16 freq = 0x284;
    const u16 pitch = 60;

    for (u16 i = DEV_CHAN_MIN_FM; i <= DEV_CHAN_MAX_FM; i++) {
        expect_synth_pitch(i, octave, freq);
        expect_synth_volume_any();
        expect_value(__wrap_synth_note_on, channel, i);

        __real_midi_note_on(0, pitch, MAX_MIDI_VOLUME);
    }

    expect_value(__wrap_synth_note_off, channel, 0);
    expect_value(__wrap_synth_note_off, channel, 1);
    expect_value(__wrap_synth_note_off, channel, 2);
    expect_value(__wrap_synth_note_off, channel, 3);
    expect_value(__wrap_synth_note_off, channel, 4);
    expect_value(__wrap_synth_note_off, channel, 5);

    __real_midi_sysex(sysExGeneralMidiResetSequence, sizeof(sysExGeneralMidiResetSequence));

    DeviceChannel* mappings = __real_midi_channel_mappings();
    for (u8 i = 0; i < DEV_CHANS; i++) {
        DeviceChannel* mapping = &mappings[i];
        assert_int_equal(mapping->midiChannel, UNASSIGNED_MIDI_CHANNEL);
        assert_int_equal(mapping->program, 0);
        assert_false(mapping->noteOn);
    }
}

void test_midi_dynamic_sends_note_off_to_channel_playing_same_pitch(UNUSED void** state)
{
    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_value(__wrap_synth_note_on, channel, 0);
    __real_midi_note_on(0, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);

    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_value(__wrap_synth_note_on, channel, 1);
    __real_midi_note_on(0, MIDI_PITCH_B6, MAX_MIDI_VOLUME);

    expect_value(__wrap_synth_note_off, channel, 1);
    __real_midi_note_off(0, MIDI_PITCH_B6);
}

void test_midi_dynamic_limits_percussion_notes(UNUSED void** state)
{
    const u8 DRUM_KEY = 30;

    for (u8 i = 0; i < 2; i++) {
        expect_synth_pitch_any();
        expect_synth_volume_any();
        expect_value(__wrap_synth_note_on, channel, i);
        expect_value(__wrap_synth_preset, channel, i);
        expect_any(__wrap_synth_preset, preset);
        __real_midi_note_on(GENERAL_MIDI_PERCUSSION_CHANNEL, DRUM_KEY, MAX_MIDI_VOLUME);
    }

    // Drum 3 (dropped)
    __real_midi_note_on(GENERAL_MIDI_PERCUSSION_CHANNEL, DRUM_KEY, MAX_MIDI_VOLUME);
}

void test_midi_dynamic_maintains_volume_on_remapping(UNUSED void** state)
{
    const u8 midi_vol = 50;
    const u8 expected_synth_vol = 0x32;

    expect_synth_volume(0, expected_synth_vol);
    __real_midi_cc(0, CC_VOLUME, midi_vol);

    // Note 1
    expect_synth_pitch_any();
    expect_synth_volume(0, expected_synth_vol);
    expect_value(__wrap_synth_note_on, channel, 0);
    __real_midi_note_on(0, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);

    // Note 2
    expect_synth_pitch_any();
    expect_synth_volume(1, expected_synth_vol);
    expect_synth_volume(1, expected_synth_vol);
    expect_value(__wrap_synth_note_on, channel, 1);
    __real_midi_note_on(0, MIDI_PITCH_B6, MAX_MIDI_VOLUME);
}

void test_midi_dynamic_sets_volume_on_playing_notes(UNUSED void** state)
{
    const u8 midi_vol_initial = 50;
    const u8 midi_vol_next = 75;
    const u8 expected_synth_vol_initial = 0x32;
    const u8 expected_synth_vol_next = 0x4b;

    expect_synth_volume(0, expected_synth_vol_initial);
    __real_midi_cc(0, CC_VOLUME, midi_vol_initial);

    // Note 1
    expect_synth_pitch_any();
    expect_synth_volume(0, expected_synth_vol_initial);
    expect_value(__wrap_synth_note_on, channel, 0);
    __real_midi_note_on(0, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);

    // Note 2
    expect_synth_pitch_any();
    expect_synth_volume(1, expected_synth_vol_initial);
    expect_synth_volume(1, expected_synth_vol_initial);
    expect_value(__wrap_synth_note_on, channel, 1);
    __real_midi_note_on(0, MIDI_PITCH_B6, MAX_MIDI_VOLUME);

    // Set volume again
    expect_synth_volume(0, expected_synth_vol_next);
    expect_synth_volume(1, expected_synth_vol_next);
    __real_midi_cc(0, CC_VOLUME, midi_vol_next);
}

void test_midi_dynamic_maintains_pan_on_remapping(UNUSED void** state)
{
    const u8 midi_pan = 127;
    const u8 expected_synth_stereo = STEREO_MODE_RIGHT;

    // Set pan
    expect_value(__wrap_synth_stereo, channel, 0);
    expect_value(__wrap_synth_stereo, mode, expected_synth_stereo);
    __real_midi_cc(0, CC_PAN, midi_pan);

    // Play note 1
    expect_synth_pitch_any();
    expect_synth_volume(0, 127);
    expect_value(__wrap_synth_note_on, channel, 0);
    __real_midi_note_on(0, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);

    // Play note 2
    expect_synth_pitch_any();
    expect_synth_volume(1, 127);
    expect_value(__wrap_synth_stereo, channel, 1);
    expect_value(__wrap_synth_stereo, mode, expected_synth_stereo);
    expect_value(__wrap_synth_note_on, channel, 1);
    __real_midi_note_on(0, MIDI_PITCH_B6, MAX_MIDI_VOLUME);
}

void test_midi_dynamic_maintains_pitch_bend_on_remapping(UNUSED void** state)
{
    const u16 midi_bend = 0x3000;

    // Note 1
    expect_synth_pitch(0, 6, 0x47a);
    expect_synth_volume(0, MAX_MIDI_VOLUME);
    expect_value(__wrap_synth_note_on, channel, 0);
    __real_midi_note_on(0, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);

    // Set bend
    expect_synth_pitch(0, 7, 0x25f);
    __real_midi_pitch_bend(0, midi_bend);

    // Note 2
    expect_synth_pitch(1, 7, 0x284);
    expect_synth_pitch(1, 7, 0x284); // triggered when pitch bend "copied"
    // note on
    expect_synth_volume(1, MAX_MIDI_VOLUME);
    expect_value(__wrap_synth_note_on, channel, 1);
    __real_midi_note_on(0, MIDI_PITCH_B6, MAX_MIDI_VOLUME);
}

void test_midi_dynamic_resets_mappings_on_cc_121(UNUSED void** state)
{
    const u8 midiChannel = 2;

    for (u16 i = DEV_CHAN_MIN_FM; i <= DEV_CHAN_MAX_FM; i++) {
        expect_synth_pitch_any();
        expect_synth_volume_any();
        expect_value(__wrap_synth_note_on, channel, i);

        __real_midi_note_on(midiChannel, MIDI_PITCH_AS6, 127);
    }

    __real_midi_cc(midiChannel, 121, 0);

    DeviceChannel* channels = __real_midi_channel_mappings();
    for (u16 i = DEV_CHAN_MIN_FM; i <= DEV_CHAN_MAX_FM; i++) {
        DeviceChannel* chan = &channels[i];
        assert_int_equal(chan->midiChannel, UNASSIGNED_MIDI_CHANNEL);
    }
}

void test_midi_dynamic_all_notes_off_on_cc_123(UNUSED void** state)
{
    const u8 midiChannel = 2;

    for (u16 i = DEV_CHAN_MIN_FM; i <= DEV_CHAN_MAX_FM; i++) {
        expect_synth_pitch_any();
        expect_synth_volume_any();
        expect_value(__wrap_synth_note_on, channel, i);

        __real_midi_note_on(midiChannel, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);
    }

    for (u16 i = DEV_CHAN_MIN_FM; i <= DEV_CHAN_MAX_FM; i++) {
        expect_value(__wrap_synth_note_off, channel, i);
    }

    __real_midi_cc(midiChannel, 123, 0);

    DeviceChannel* channels = __real_midi_channel_mappings();
    for (u16 i = DEV_CHAN_MIN_FM; i <= DEV_CHAN_MAX_FM; i++) {
        DeviceChannel* chan = &channels[i];
        assert_false(chan->noteOn);
    }
}

void test_midi_dynamic_sysex_remaps_midi_channel(UNUSED void** state)
{
    const u8 MIDI_CHANNEL = 0x01;
    const u8 DESTINATION_FIRST_PSG_CHANNEL = 0x06;

    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_REMAP, MIDI_CHANNEL, DESTINATION_FIRST_PSG_CHANNEL };

    __real_midi_sysex(sequence, sizeof(sequence));

    expect_any_psg_tone_on_channel(0);
    expect_psg_attenuation(0, PSG_ATTENUATION_LOUDEST);

    __real_midi_note_on(MIDI_CHANNEL, 60, MAX_MIDI_VOLUME);
}

void test_midi_dynamic_sysex_removes_mapping_of_midi_channel(UNUSED void** state)
{
    const u8 MIDI_CHANNEL = 0x00;
    const u8 MIDI_CHANNEL_NO_MAPPING = 0x7F;
    const u8 DESTINATION_FIRST_PSG_CHANNEL = 0x06;

    const u8 sequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_REMAP, MIDI_CHANNEL, DESTINATION_FIRST_PSG_CHANNEL };

    // Assigning PSG to MIDI Chan 0
    __real_midi_sysex(sequence, sizeof(sequence));

    // Play note
    expect_any_psg_tone_on_channel(0);
    expect_psg_attenuation(0, PSG_ATTENUATION_LOUDEST);
    __real_midi_note_on(MIDI_CHANNEL, 60, MAX_MIDI_VOLUME);

    const u8 removeMappingSeq[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID,
        SYSEX_COMMAND_REMAP, MIDI_CHANNEL_NO_MAPPING, DESTINATION_FIRST_PSG_CHANNEL };

    // Remove MIDI Chan 0 assignment
    __real_midi_sysex(removeMappingSeq, sizeof(removeMappingSeq));

    // Play note
    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_value(__wrap_synth_note_on, channel, 0);
    __real_midi_note_on(MIDI_CHANNEL, 60, MAX_MIDI_VOLUME);
}

void test_midi_dynamic_prefers_psg_for_square_wave_instruments(UNUSED void** state)
{
    const u8 PSG_CHANNEL = 0;
    const u8 MIDI_CHANNEL = 0;
    const u8 SQUARE_WAVE_MIDI_PROGRAMS[3] = { 80, 89, 99 };

    for (u8 i = 0; i < LENGTH_OF(SQUARE_WAVE_MIDI_PROGRAMS); i++) {
        u8 program = SQUARE_WAVE_MIDI_PROGRAMS[i];

        __real_midi_program(MIDI_CHANNEL, program);

        if (i == 0) {
            expect_any_psg_tone_on_channel(0);
        }
        expect_psg_attenuation(PSG_CHANNEL, PSG_ATTENUATION_LOUDEST);
        __real_midi_note_on(MIDI_CHANNEL, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);

        expect_psg_attenuation(PSG_CHANNEL, PSG_ATTENUATION_SILENCE);
        __real_midi_note_off(MIDI_CHANNEL, MIDI_PITCH_AS6);
    }
}

void test_midi_dynamic_sticks_to_assigned_device_type_for_midi_channels(UNUSED void** state)
{
    setStickToDeviceType(true);

    const u16 pitch = MIDI_PITCH_B6;
    const u8 REUSE_MIDI_CHANNEL = 2;

    for (u8 chan = DEV_CHAN_MIN_FM; chan <= DEV_CHAN_MAX_FM; chan++) {
        expect_synth_pitch_any();
        expect_synth_volume_any();
        expect_value(__wrap_synth_note_on, channel, chan);
        __real_midi_note_on(REUSE_MIDI_CHANNEL, pitch, MAX_MIDI_VOLUME);
    }

    expect_synth_pitch(0, 6, 0x47a);
    expect_synth_volume_any();
    expect_value(__wrap_synth_note_on, channel, 0);
    __real_midi_note_on(REUSE_MIDI_CHANNEL, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);
}

void test_midi_dynamic_sticks_to_assigned_psg_device_type_for_midi_channels(UNUSED void** state)
{
    setStickToDeviceType(true);

    const u8 MIDI_CHANNEL = 0;
    const u8 SQUARE_WAVE_MIDI_PROGRAM = 80;

    __real_midi_program(MIDI_CHANNEL, SQUARE_WAVE_MIDI_PROGRAM);

    for (u8 chan = DEV_CHAN_MIN_PSG; chan <= DEV_CHAN_MAX_TONE_PSG; chan++) {
        u8 psgChannel = chan - DEV_CHAN_MIN_PSG;
        expect_any_psg_tone_on_channel(psgChannel);
        expect_psg_attenuation(psgChannel, PSG_ATTENUATION_LOUDEST);
        __real_midi_note_on(MIDI_CHANNEL, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);
    }

    expect_any_psg_tone_on_channel(0);
    __real_midi_note_on(MIDI_CHANNEL, MIDI_PITCH_B6, MAX_MIDI_VOLUME);
}

void test_midi_assign_channel_to_psg_device(UNUSED void** state)
{
    const u8 MIDI_CHANNEL = 0;
    const u8 PSG_CHANNEL = 0;
    const u8 DEVICE_SELECT_PSG_TONE = 64;

    __real_midi_cc(MIDI_CHANNEL, 86, DEVICE_SELECT_PSG_TONE);

    expect_any_psg_tone_on_channel(PSG_CHANNEL);
    expect_psg_attenuation(PSG_CHANNEL, PSG_ATTENUATION_LOUDEST);

    __real_midi_note_on(MIDI_CHANNEL, 60, MAX_MIDI_VOLUME);
}

void test_midi_assign_channel_to_fm_device_only(UNUSED void** state)
{
    const u8 MIDI_CHANNEL = 0;
    const u8 DEVICE_SELECT_FM = 32;

    __real_midi_cc(MIDI_CHANNEL, 86, DEVICE_SELECT_FM);

    for (u8 chan = DEV_CHAN_MIN_FM; chan <= DEV_CHAN_MAX_FM; chan++) {
        expect_synth_pitch_any();
        expect_synth_volume_any();
        expect_value(__wrap_synth_note_on, channel, chan);
        __real_midi_note_on(MIDI_CHANNEL, MIDI_PITCH_AS6, MAX_MIDI_VOLUME);
    }

    expect_synth_pitch_any();
    expect_synth_volume_any();
    expect_value(__wrap_synth_note_on, channel, 0);
    __real_midi_note_on(MIDI_CHANNEL, 60, MAX_MIDI_VOLUME);
}

void test_midi_assign_channel_to_psg_noise(UNUSED void** state)
{
    const u8 MIDI_CHANNEL = 0;
    const u8 PSG_NOISE_CHANNEL = 3;
    const u8 DEVICE_SELECT_PSG_NOISE = 96;

    __real_midi_cc(MIDI_CHANNEL, 86, DEVICE_SELECT_PSG_NOISE);

    expect_any_psg_tone_on_channel(PSG_NOISE_CHANNEL);
    expect_psg_attenuation(PSG_NOISE_CHANNEL, PSG_ATTENUATION_LOUDEST);
    __real_midi_note_on(MIDI_CHANNEL, 60, MAX_MIDI_VOLUME);
}
