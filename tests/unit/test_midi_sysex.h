#include "cmocka_inc.h"

void test_midi_sysex_sends_all_notes_off(UNUSED void** state);
void test_midi_sysex_general_midi_reset_resets_synth_volume(UNUSED void** state);
void test_midi_sysex_ignores_unknown_sysex(UNUSED void** state);
void test_midi_sysex_remaps_midi_channel_to_psg(UNUSED void** state);
void test_midi_sysex_remaps_midi_channel_to_fm(UNUSED void** state);
void test_midi_sysex_unassigns_midi_channel(UNUSED void** state);
void test_midi_sysex_does_nothing_for_empty_payload(UNUSED void** state);
void test_midi_sysex_handles_incomplete_channel_mapping_command(UNUSED void** state);
void test_midi_sysex_enables_dynamic_channel_mode(UNUSED void** state);
void test_midi_sysex_sets_mapping_mode_to_auto(UNUSED void** state);
void test_midi_sysex_disables_fm_parameter_CCs(UNUSED void** state);
void test_midi_sysex_loads_psg_envelope(UNUSED void** state);
void test_midi_sysex_inverts_total_level_values(UNUSED void** state);
void test_midi_sysex_sets_original_total_level_values(UNUSED void** state);
void test_midi_sysex_writes_directly_to_ym2612_regs_part_0(UNUSED void** state);
void test_midi_sysex_writes_directly_to_ym2612_regs_part_1(UNUSED void** state);
void test_midi_sysex_ignores_incorrect_length_ym2612_direct_writes(UNUSED void** state);
void test_midi_sysex_stores_program(UNUSED void** state);
void test_midi_sysex_logs_warning_if_program_store_length_is_incorrect(UNUSED void** state);
void test_midi_sysex_logs_warning_if_program_store_type_is_incorrect(UNUSED void** state);
void test_midi_sysex_clears_program(UNUSED void** state);
void test_midi_sysex_logs_warning_if_program_clear_length_is_incorrect(UNUSED void** state);
void test_midi_sysex_logs_warning_if_program_clear_type_is_incorrect(UNUSED void** state);