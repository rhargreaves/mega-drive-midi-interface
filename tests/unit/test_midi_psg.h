#include "cmocka_inc.h"

void test_midi_triggers_psg_note_on(UNUSED void** state);
void test_midi_uses_PAL_tones_if_system_is_in_that_region(UNUSED void** state);
void test_midi_triggers_psg_note_on_with_velocity(UNUSED void** state);
void test_midi_triggers_psg_note_on_with_velocity_and_channel_volume(UNUSED void** state);
void test_midi_changing_volume_during_psg_note_on_respects_velocity(UNUSED void** state);
void test_midi_triggers_psg_note_off(UNUSED void** state);
void test_midi_drops_psg_key_below_45(UNUSED void** state);
void test_midi_triggers_psg_note_off_and_volume_change_does_not_cause_psg_channel_to_play(
    UNUSED void** state);
void test_midi_psg_note_off_only_triggered_if_specific_note_is_on(UNUSED void** state);
void test_midi_channel_volume_sets_psg_attenuation(UNUSED void** state);
void test_midi_channel_volume_sets_psg_attenuation_2(UNUSED void** state);
void test_midi_sets_psg_pitch_bend_down(UNUSED void** state);
void test_midi_sets_psg_pitch_bend_up(UNUSED void** state);
void test_midi_psg_pitch_bend_persists_after_tick(UNUSED void** state);
void test_midi_plays_psg_envelope(UNUSED void** state);
void test_midi_plays_advanced_psg_envelope(UNUSED void** state);
void test_midi_loops_psg_envelope(UNUSED void** state);
void test_midi_psg_envelope_with_only_end_flag_is_silent(UNUSED void** state);
void test_midi_psg_envelope_with_loop_end_continues_playing_after_note_off(UNUSED void** state);
void test_midi_psg_envelope_with_loop_end_resets_release_note_after_note_silenced(
    UNUSED void** state);
void test_midi_shifts_semitone_in_psg_envelope(UNUSED void** state);
void test_midi_pitch_shift_handles_upper_limit_psg_envelope(UNUSED void** state);
void test_midi_pitch_shift_handles_lower_limit_psg_envelope(UNUSED void** state);
void test_midi_loads_psg_envelope(UNUSED void** state);
void test_midi_psg_sets_busy_indicators(UNUSED void** state);
