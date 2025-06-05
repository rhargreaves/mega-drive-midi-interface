#include "cmocka_inc.h"

int test_e2e_setup(void** state);
void test_midi_note_on_event_sent_to_ym2612(void** state);
void test_polyphonic_midi_sent_to_separate_ym2612_channels(void** state);
void test_psg_audible_if_note_on_event_triggered(void** state);
void test_psg_not_audible_if_midi_channel_volume_set_and_there_is_no_note_on_event(void** state);
void test_general_midi_reset_sysex_stops_all_notes(void** state);
void test_remap_midi_channel_1_to_psg_channel_1();
void test_set_device_for_midi_channel_1_to_psg();
void test_pong_received_after_ping_sent();
void test_loads_psg_envelope();
void test_enables_ch3_special_mode(void** state);
void test_sets_separate_ch3_operator_frequencies(void** state);
void test_write_directly_to_ym2612_regs_via_sysex(void** state);
void test_plays_pcm_sample(void** state);
void test_midi_last_note_played_priority_respected_on_fm(void** state);
void test_midi_last_note_played_remembers_velocity_on_fm(void** state);
void test_midi_last_note_played_cleared_when_released_on_fm(void** state);
void test_midi_changing_program_retains_pan(void** state);
void test_midi_changing_program_retains_volume(void** state);
void test_midi_portamento_glides_note(void** state);
void test_pitch_bends_ch3_special_mode_operators(void** state);
void test_midi_pitch_bend_range_configurable_per_channel(void** state);
void test_dump_preset_to_callee(void** state);
void test_dump_channel_parameters_to_callee(void** state);
