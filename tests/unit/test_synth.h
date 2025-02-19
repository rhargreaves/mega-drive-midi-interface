#include "cmocka_inc.h"

#define SYNTH_VOLUME_MAX 0x7F

int test_synth_setup(UNUSED void** state);
void test_synth_init_sets_initial_registers(UNUSED void** state);
void test_synth_sets_note_on_fm_reg_chan_0_to_2(UNUSED void** state);
void test_synth_sets_note_on_fm_reg_chan_3_to_5(UNUSED void** state);
void test_synth_sets_note_off_fm_reg_chan_0_to_2(UNUSED void** state);
void test_synth_sets_note_off_fm_reg_chan_3_to_5(UNUSED void** state);
void test_synth_sets_octave_and_freq_reg_chan(UNUSED void** state);
void test_synth_sets_stereo_ams_and_freq(UNUSED void** state);
void test_synth_sets_algorithm(UNUSED void** state);
void test_synth_sets_feedback(UNUSED void** state);
void test_synth_sets_feedback_and_algorithm(UNUSED void** state);
void test_synth_does_not_reset_operator_level_if_equal(UNUSED void** state);
void test_synth_sets_operator_total_level(UNUSED void** state);
void test_synth_sets_operator_multiple_and_detune(UNUSED void** state);
void test_synth_sets_operator_attack_rate_and_rate_scaling(UNUSED void** state);
void test_synth_sets_operator_sustain_rate(UNUSED void** state);
void test_synth_sets_operator_release_rate_and_sustain_level(UNUSED void** state);
void test_synth_sets_operator_amplitude_modulation_and_decay_rate(UNUSED void** state);
void test_synth_sets_operator_ssg_eg(UNUSED void** state);
void test_synth_sets_global_LFO_enable_and_frequency(UNUSED void** state);
void test_synth_sets_busy_indicators(UNUSED void** state);
void test_synth_sets_preset(UNUSED void** state);
void test_synth_sets_preset_retaining_pan(UNUSED void** state);
void test_synth_applies_volume_modifier_to_output_operators_algorithm_7(UNUSED void** state);
void test_synth_does_not_apply_volume_if_equal(UNUSED void** state);
void test_synth_applies_volume_modifier_to_output_operators_algorithm_7_quieter(
    UNUSED void** state);
void test_synth_applies_volume_modifier_to_output_operators_algorithms_0_to_3(UNUSED void** state);
void test_synth_applies_volume_modifier_to_output_operators_algorithm_4(UNUSED void** state);
void test_synth_applies_volume_modifier_to_output_operators_algorithms_5_and_6(UNUSED void** state);
void test_synth_exposes_fm_channel_parameters(UNUSED void** state);
void test_synth_exposes_global_parameters(UNUSED void** state);
void test_synth_calls_callback_when_parameter_changes(UNUSED void** state);
void test_synth_calls_callback_when_lfo_freq_changes(UNUSED void** state);
void test_synth_calls_callback_when_lfo_enable_changes(UNUSED void** state);
void test_synth_calls_callback_when_special_mode_changes(UNUSED void** state);
void test_synth_enables_ch3_special_mode(UNUSED void** state);
void test_synth_disables_ch3_special_mode(UNUSED void** state);
void test_synth_sets_ch3_special_mode_operator_pitches(UNUSED void** state);
void test_synth_handles_out_of_range_ch3_special_mode_operator(UNUSED void** state);
void test_synth_sets_ch3_special_mode_op_tl_only_if_output_operator(UNUSED void** state);
void test_synth_enables_dac(UNUSED void** state);
void test_synth_disables_dac(UNUSED void** state);
void test_requests_Z80_bus_if_not_already_taken(UNUSED void** state);
void test_does_not_release_Z80_bus_when_taken_prior_to_call(UNUSED void** state);
