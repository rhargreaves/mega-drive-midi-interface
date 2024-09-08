#include "cmocka_inc.h"

int test_dynamic_midi_setup(UNUSED void** state);
void test_midi_dynamic_uses_all_channels(UNUSED void** state);
void test_midi_routing_switches_to_dynamic_on_gm_reset(UNUSED void** state);
void test_midi_dynamic_tries_to_reuse_original_midi_channel_if_available(UNUSED void** state);
void test_midi_dynamic_reuses_mapped_midi_channel_even_if_busy_if_sticking_to_device_type(
    UNUSED void** state);
void test_midi_reports_dynamic_mode_enabled(UNUSED void** state);
void test_midi_reports_dynamic_mode_disabled(UNUSED void** state);
void test_midi_exposes_dynamic_mode_mappings(UNUSED void** state);
void test_midi_dynamic_enables_percussive_mode_if_needed(UNUSED void** state);
void test_midi_sets_presets_on_dynamic_channels(UNUSED void** state);
void test_midi_dynamic_does_not_send_percussion_to_psg_channels(UNUSED void** state);
void test_midi_sysex_resets_dynamic_mode_state(UNUSED void** state);
void test_midi_dynamic_sends_note_off_to_channel_playing_same_pitch(UNUSED void** state);
void test_midi_dynamic_limits_percussion_notes(UNUSED void** state);
void test_midi_dynamic_maintains_volume_on_remapping(UNUSED void** state);
void test_midi_dynamic_sets_volume_on_playing_notes(UNUSED void** state);
void test_midi_dynamic_maintains_pan_on_remapping(UNUSED void** state);
void test_midi_dynamic_maintains_pitch_bend_on_remapping(UNUSED void** state);
void test_midi_dynamic_resets_mappings_on_cc_121(UNUSED void** state);
void test_midi_dynamic_all_notes_off_on_cc_123(UNUSED void** state);
void test_midi_dynamic_sysex_remaps_midi_channel(UNUSED void** state);
void test_midi_dynamic_sysex_removes_mapping_of_midi_channel(UNUSED void** state);
void test_midi_dynamic_prefers_psg_for_square_wave_instruments(UNUSED void** state);
void test_midi_dynamic_sticks_to_assigned_device_type_for_midi_channels(UNUSED void** state);
void test_midi_dynamic_sticks_to_assigned_psg_device_type_for_midi_channels(UNUSED void** state);
void test_midi_assign_channel_to_psg_device(UNUSED void** state);
void test_midi_assign_channel_to_fm_device_only(UNUSED void** state);
void test_midi_assign_channel_to_psg_noise(UNUSED void** state);
