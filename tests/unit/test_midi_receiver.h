#include "cmocka_inc.h"

int test_midi_receiver_setup(UNUSED void** state);
void test_midi_receiver_initialises(UNUSED void** state);
void test_midi_receiver_read_passes_note_on_to_midi_processor(UNUSED void** state);
void test_midi_receiver_read_passes_note_off_to_midi_processor(UNUSED void** state);
void test_midi_receiver_does_nothing_for_control_change(UNUSED void** state);
void test_midi_receiver_sets_unknown_event_for_unknown_status(UNUSED void** state);
void test_midi_receiver_sets_unknown_event_for_unknown_system_message(UNUSED void** state);
void test_midi_receiver_sets_CC(UNUSED void** state);
void test_midi_receiver_sets_pitch_bend(UNUSED void** state);
void test_midi_receiver_does_nothing_on_midi_clock(UNUSED void** state);
void test_midi_receiver_does_nothing_on_midi_start_midi(UNUSED void** state);
void test_midi_receiver_swallows_midi_stop(UNUSED void** state);
void test_midi_receiver_swallows_midi_continue(UNUSED void** state);
void test_midi_receiver_does_nothing_on_midi_position(UNUSED void** state);
void test_midi_receiver_sets_midi_program(UNUSED void** state);
void test_midi_receiver_sends_midi_reset(UNUSED void** state);
void test_midi_receiver_sends_sysex_to_midi_layer(UNUSED void** state);
void test_midi_receiver_handles_sysex_limits(UNUSED void** state);
