#include "cmocka_inc.h"

int test_midi_portamento_setup(UNUSED void** state);
void test_midi_portamento_glides_note_up(UNUSED void** state);
void test_midi_portamento_glides_note_down(UNUSED void** state);
void test_midi_portamento_glides_note_up_and_down_on_early_release(UNUSED void** state);
void test_midi_portamento_glide_ignores_unassigned_channels(UNUSED void** state);
void test_midi_portamento_glides_note_up_down_and_back_up(UNUSED void** state);
void test_midi_portamento_glides_only_if_target_set(UNUSED void** state);
void test_midi_portamento_glide_ends_after_both_notes_off(UNUSED void** state);
void test_midi_portamento_glides_fully_up_and_down(UNUSED void** state);
void test_midi_portamento_synth_note_off_triggered(UNUSED void** state);
void test_midi_portamento_zeros_any_residual_cents(UNUSED void** state);
void test_midi_portamento_glides_note_up_for_psg(UNUSED void** state);
void test_midi_portamento_glides_note_up_with_pitch_bend(UNUSED void** state);
void test_midi_portamento_glides_note_down_with_pitch_bend(UNUSED void** state);
void test_midi_portamento_sets_portamento_time_to_minimum(UNUSED void** state);
void test_midi_portamento_sets_portamento_time_to_maximum(UNUSED void** state);
void test_midi_portamento_default_portamento_time_set(UNUSED void** state);
void test_midi_portamento_glides_with_fine_tune(UNUSED void** state);
