#include "test_midi.h"
#include "wraps.h"
#include "mocks/mock_midi.h"

static const FmChannel M_BANK_0_INST_0_GRANDPIANO = { 2, 0, 3, 0, 0, 0, 0,
    { { 1, 0, 26, 1, 7, 0, 7, 4, 1, 39, 0 }, { 4, 6, 24, 1, 9, 0, 6, 9, 7, 36, 0 },
        { 2, 7, 31, 3, 23, 0, 9, 15, 1, 4, 0 }, { 1, 3, 27, 2, 4, 0, 10, 4, 6, 2, 0 } } };

static const FmChannel M_BANK_0_INST_1_BRIGHTPIANO = { 5, 7, 3, 0, 0, 0, 0,
    { { 4, 2, 27, 1, 9, 0, 11, 5, 6, 33, 0 }, { 4, 5, 27, 1, 9, 0, 7, 9, 7, 18, 0 },
        { 1, 2, 27, 1, 5, 1, 10, 5, 6, 8, 0 }, { 6, 5, 27, 1, 9, 0, 3, 8, 7, 9, 0 } } };

static const PercussionPreset P_BANK_0_INST_30_CASTANETS = {
    { 4, 3, 3, 0, 0, 0, 0,
        { { 9, 0, 31, 0, 11, 0, 15, 0, 15, 23, 0 }, { 1, 0, 31, 0, 19, 0, 15, 0, 15, 15, 0 },
            { 4, 0, 31, 2, 20, 0, 15, 0, 15, 13, 0 }, { 2, 0, 31, 2, 20, 0, 15, 0, 15, 13, 0 } } },
    0
};

static const FmChannel* M_BANK_0[MIDI_PROGRAMS]
    = { &M_BANK_0_INST_0_GRANDPIANO, &M_BANK_0_INST_1_BRIGHTPIANO };

static const PercussionPreset* P_BANK_0[MIDI_PROGRAMS];

static const u8 ENVELOPE_0[] = { 0x00, EEF_LOOP_START, 0x00, EEF_END };
static const u8 ENVELOPE_1[] = { 0x00, 0x0F, EEF_END };
static const u8 ENVELOPE_2[] = { 0x00, 0x07, 0x0F, EEF_END };
static const u8 ENVELOPE_3[] = { 0x00, 0x01, 0x02, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x04, 0x04,
    0x08, 0x08, 0x08, 0x08, 0x0A, 0x0A, 0x0C, 0x0C, EEF_END };
static const u8 ENVELOPE_4[] = { EEF_LOOP_START, 0x00, 0x07, EEF_END };
static const u8 ENVELOPE_5[] = { EEF_END };
static const u8 ENVELOPE_6[] = { EEF_LOOP_START, 0x00, EEF_LOOP_END, 0x05, EEF_END };
static const u8 ENVELOPE_7[] = { EEF_LOOP_START, 0x00, 0x10, EEF_END };
static const u8 ENVELOPE_8[] = { 0x00, 0x10, 0x20, 0x30, 0x40, EEF_END };

static const u8* TEST_ENVELOPES[MIDI_PROGRAMS] = { ENVELOPE_0, ENVELOPE_1, ENVELOPE_2, ENVELOPE_3,
    ENVELOPE_4, ENVELOPE_5, ENVELOPE_6, ENVELOPE_7, ENVELOPE_8, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0 };

int test_midi_setup(UNUSED void** state)
{
    expect_any(__wrap_scheduler_addFrameHandler, onFrame);
    expect_any(__wrap_scheduler_addFrameHandler, onFrame);

    P_BANK_0[30] = &P_BANK_0_INST_30_CASTANETS;
    mock_log_disable_checks();
    wraps_disable_checks();
    mock_synth_disable_checks();
    midi_init(M_BANK_0, P_BANK_0, TEST_ENVELOPES);
    wraps_enable_checks();
    mock_synth_enable_checks();
    wraps_set_SYS_isPAL(false);
    return 0;
}

void test_midi_polyphonic_mode_returns_state(UNUSED void** state)
{
    __real_midi_cc(0, CC_GENMDM_POLYPHONIC_MODE, 127);

    assert_true(__real_midi_dynamic_mode());

    __real_midi_cc(0, CC_GENMDM_POLYPHONIC_MODE, 0);
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
    mock_log_enable_checks();

    u8 expectedController = 0x9;
    u8 expectedValue = 0x50;

    expect_log_warn("Ch %d: CC 0x%02X 0x%02X?");
    __real_midi_cc(0, expectedController, expectedValue);
}

void test_midi_ignores_sustain_pedal_cc(UNUSED void** state)
{
    mock_log_enable_checks();

    u8 cc = 64;

    __real_midi_cc(0, cc, 1);
}

void test_midi_ignores_expression_cc(UNUSED void** state)
{
    mock_log_enable_checks();

    u8 cc = 11;

    __real_midi_cc(0, cc, 1);
}

void test_midi_ignores_sysex_nrpn_ccs(UNUSED void** state)
{
    mock_log_enable_checks();

    __real_midi_cc(0, 6, 1);
    __real_midi_cc(0, 38, 1);
    __real_midi_cc(0, 98, 1);
    __real_midi_cc(0, 99, 1);
    __real_midi_cc(0, 100, 1);
    __real_midi_cc(0, 101, 1);
}

void test_midi_shows_fm_parameter_ui(UNUSED void** state)
{
    u8 midiChan = 0;
    u8 cc = 83;
    u8 show = 127;

    expect_value(__wrap_ui_fm_set_parameters_visibility, chan, midiChan);
    expect_value(__wrap_ui_fm_set_parameters_visibility, show, true);

    __real_midi_cc(midiChan, cc, show);
}

void test_midi_hides_fm_parameter_ui(UNUSED void** state)
{
    u8 midiChan = 0;
    u8 cc = 83;
    u8 hide = 0;

    expect_value(__wrap_ui_fm_set_parameters_visibility, chan, midiChan);
    expect_value(__wrap_ui_fm_set_parameters_visibility, show, false);

    __real_midi_cc(midiChan, cc, hide);
}

void test_midi_resets_fm_values_to_defaults(UNUSED void** state)
{
    for (u8 ch = 0; ch < 6; ch++) {
        expect_value(__wrap_synth_volume, channel, ch);
        expect_value(__wrap_synth_volume, volume, 60);

        __real_midi_cc(ch, CC_VOLUME, 60);

        DeviceChannel* chans = __real_midi_channel_mappings();
        assert_int_equal(chans[ch].volume, 60);
    }

    expect_any(__wrap_synth_init, defaultPreset);

    for (u8 ch = 0; ch < 6; ch++) {
        expect_value(__wrap_synth_volume, channel, ch);
        expect_value(__wrap_synth_volume, volume, 127);
    }

    __real_midi_reset();

    for (u8 ch = 0; ch < 6; ch++) {
        DeviceChannel* chans = __real_midi_channel_mappings();
        assert_int_equal(chans[ch].volume, 127);
    }
}

void test_midi_resets_psg_values_to_defaults(UNUSED void** state)
{
    for (u8 ch = 6; ch < 10; ch++) {
        __real_midi_cc(ch, CC_VOLUME, 60);

        DeviceChannel* chans = __real_midi_channel_mappings();
        assert_int_equal(chans[ch].volume, 60);
    }

    expect_any(__wrap_synth_init, defaultPreset);

    __real_midi_reset();

    for (u8 ch = 6; ch < 10; ch++) {
        DeviceChannel* chans = __real_midi_channel_mappings();
        assert_int_equal(chans[ch].volume, 127);
    }
}
