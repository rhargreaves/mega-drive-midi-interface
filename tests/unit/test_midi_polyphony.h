#include "cmocka_inc.h"

void test_midi_polyphonic_mode_sends_CCs_to_all_FM_channels(UNUSED void** state);
void test_midi_set_overflow_flag_on_polyphony_breach(UNUSED void** state);
void test_midi_polyphonic_mode_uses_multiple_fm_channels(UNUSED void** state);
void test_midi_polyphonic_mode_note_off_silences_all_matching_pitch(UNUSED void** state);
void test_midi_sets_all_notes_off_in_polyphonic_mode(UNUSED void** state);
void test_midi_sets_polyphonic_mode(UNUSED void** state);
void test_midi_unsets_polyphonic_mode(UNUSED void** state);
void test_midi_sets_all_channel_mappings_when_setting_polyphonic_mode(UNUSED void** state);
