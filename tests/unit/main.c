#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "test_comm.c"
#include "test_midi.c"
#include "test_midi_fm.c"
#include "test_midi_polyphony.c"
#include "test_midi_psg.c"
#include "test_midi_receiver.c"
#include "test_midi_sysex.c"
#include "test_midi_timing.c"
#include "test_psg_chip.c"
#include "test_sonic.c"
#include "test_synth.c"
#include <cmocka.h>

#define midi_test(test) cmocka_unit_test_setup(test, test_midi_setup)
#define synth_test(test) cmocka_unit_test_setup(test, test_synth_setup)
#define comm_test(test) cmocka_unit_test_setup(test, test_comm_setup)
#define sonic_test(test) cmocka_unit_test_setup(test, test_sonic_setup)

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(
            test_midi_receiver_read_passes_note_on_to_midi_processor),
        cmocka_unit_test(
            test_midi_receiver_read_passes_note_off_to_midi_processor),
        cmocka_unit_test(test_midi_receiver_does_nothing_for_control_change),
        cmocka_unit_test(
            test_midi_receiver_sets_unknown_event_for_unknown_status),
        cmocka_unit_test(
            test_midi_receiver_sets_unknown_event_for_unknown_system_message),
        cmocka_unit_test(test_midi_receiver_sets_CC),
        cmocka_unit_test(test_midi_receiver_sets_pitch_bend),
        cmocka_unit_test(test_midi_receiver_increments_midi_clock),
        cmocka_unit_test(test_midi_receiver_starts_midi),
        cmocka_unit_test(test_midi_receiver_swallows_midi_stop),
        cmocka_unit_test(test_midi_receiver_swallows_midi_continue),
        cmocka_unit_test(test_midi_receiver_sets_position),
        cmocka_unit_test(test_midi_receiver_sets_midi_program),
        cmocka_unit_test(test_midi_receiver_sends_sysex_to_midi_layer),

        midi_test(test_midi_triggers_synth_note_on),
        midi_test(test_midi_triggers_synth_note_on_with_velocity),
        midi_test(
            test_midi_triggers_synth_note_on_with_velocity_and_channel_volume),
        midi_test(test_midi_triggers_synth_note_on_boundary_values),
        midi_test(test_midi_does_not_trigger_synth_note_on_out_of_bound_values),
        midi_test(test_midi_triggers_synth_note_on_2),
        midi_test(test_midi_triggers_synth_note_off),
        midi_test(test_midi_triggers_psg_note_on),
        midi_test(test_midi_triggers_psg_note_on_with_velocity),
        midi_test(
            test_midi_triggers_psg_note_on_with_velocity_and_channel_volume),
        midi_test(
            test_midi_changing_volume_during_psg_note_on_respects_velocity),
        midi_test(test_midi_changing_volume_during_note_on_respects_velocity),
        midi_test(test_midi_triggers_psg_note_off),
        midi_test(test_midi_drops_psg_key_below_45),
        midi_test(test_midi_psg_note_off_only_triggered_if_specific_note_is_on),
        midi_test(
            test_midi_triggers_psg_note_off_and_volume_change_does_not_cause_psg_channel_to_play),
        midi_test(test_midi_channel_volume_sets_volume),
        midi_test(test_midi_pan_sets_synth_stereo_mode_right),
        midi_test(test_midi_pan_sets_synth_stereo_mode_left),
        midi_test(test_midi_pan_sets_synth_stereo_mode_centre),
        midi_test(test_midi_channel_volume_sets_psg_attenuation),
        midi_test(test_midi_channel_volume_sets_psg_attenuation_2),
        midi_test(test_midi_directs_channels_above_10_to_psg),
        midi_test(test_midi_sets_synth_pitch_bend),
        midi_test(test_midi_sets_psg_pitch_bend),
        midi_test(test_midi_polyphonic_mode_returns_state),
        midi_test(test_midi_polyphonic_mode_uses_multiple_fm_channels),
        midi_test(
            test_midi_polyphonic_mode_note_off_silences_all_matching_pitch),
        midi_test(test_midi_sets_all_notes_off),
        midi_test(test_midi_sets_all_notes_off_in_polyphonic_mode),
        midi_test(test_midi_sets_fm_algorithm),
        midi_test(test_midi_sets_fm_feedback),
        midi_test(test_midi_sets_operator_total_level),
        midi_test(test_midi_sets_operator_multiple),
        midi_test(test_midi_sets_operator_detune),
        midi_test(test_midi_sets_operator_rate_scaling),
        midi_test(test_midi_sets_operator_attack_rate),
        midi_test(test_midi_sets_operator_first_decay_rate),
        midi_test(test_midi_sets_operator_second_decay_rate),
        midi_test(test_midi_sets_operator_secondary_amplitude),
        midi_test(test_midi_sets_operator_amplitude_modulation),
        midi_test(test_midi_sets_operator_release_rate),
        midi_test(test_midi_sets_operator_ssg_eg),
        midi_test(test_midi_sets_global_LFO_enable),
        midi_test(test_midi_sets_global_LFO_frequency),
        midi_test(test_midi_sets_channel_AMS),
        midi_test(test_midi_sets_channel_FMS),
        midi_test(test_midi_sets_polyphonic_mode),
        midi_test(test_midi_unsets_polyphonic_mode),
        midi_test(test_midi_sets_unknown_CC),
        midi_test(test_midi_polyphonic_mode_sends_CCs_to_all_FM_channels),
        midi_test(test_midi_set_overflow_flag_on_polyphony_breach),
        midi_test(test_midi_clears_overflow_flag),
        midi_test(test_midi_increments_beat_every_24th_clock),
        midi_test(test_midi_start_resets_clock),
        midi_test(test_midi_position_sets_correct_timing),
        midi_test(test_midi_timing_sets_bar_number),
        midi_test(test_midi_increments_clocks),
        midi_test(test_midi_sets_fm_preset),
        midi_test(test_midi_sysex_sends_all_notes_off),
        midi_test(test_midi_sysex_ignores_unknown_sysex),
        midi_test(test_midi_sysex_remaps_midi_channel_to_psg),
        midi_test(test_midi_sysex_remaps_midi_channel_to_fm),
        midi_test(test_midi_sysex_unassigns_midi_channel),
        midi_test(test_midi_sysex_does_nothing_for_empty_payload),
        midi_test(test_midi_sysex_handles_incomplete_channel_mapping_command),
        midi_test(
            test_midi_fm_note_on_percussion_channel_sets_percussion_preset),
        midi_test(test_midi_switching_program_retains_pan_setting),
        midi_test(test_midi_sets_genmdm_stereo_mode),
        midi_test(test_midi_exposes_channel_mappings),

        synth_test(test_synth_init_sets_initial_registers),
        synth_test(test_synth_sets_note_on_fm_reg_chan_0_to_2),
        synth_test(test_synth_sets_note_on_fm_reg_chan_3_to_5),
        synth_test(test_synth_sets_note_off_fm_reg_chan_0_to_2),
        synth_test(test_synth_sets_note_off_fm_reg_chan_3_to_5),
        synth_test(test_synth_sets_octave_and_freq_reg_chan),
        synth_test(test_synth_sets_stereo_ams_and_freq),
        synth_test(test_synth_sets_algorithm),
        synth_test(test_synth_sets_feedback),
        synth_test(test_synth_sets_operator_total_level),
        synth_test(test_synth_sets_feedback_and_algorithm),
        synth_test(test_synth_sets_operator_multiple_and_detune),
        synth_test(test_synth_sets_operator_attack_rate_and_rate_scaling),
        synth_test(test_synth_sets_operator_second_decay_rate),
        synth_test(
            test_synth_sets_operator_release_rate_and_secondary_amplitude),
        synth_test(
            test_synth_sets_operator_amplitude_modulation_and_first_decay_rate),
        synth_test(test_synth_sets_operator_ssg_eg),
        synth_test(test_synth_sets_global_LFO_enable_and_frequency),
        synth_test(test_synth_sets_busy_indicators),
        synth_test(test_synth_sets_preset),
        synth_test(
            test_synth_applies_volume_modifier_to_output_operators_algorithm_7),
        synth_test(
            test_synth_applies_volume_modifier_to_output_operators_algorithm_7_quieter),
        synth_test(
            test_synth_applies_volume_modifier_to_output_operators_algorithms_0_to_3),
        synth_test(
            test_synth_applies_volume_modifier_to_output_operators_algorithm_4),
        synth_test(
            test_synth_applies_volume_modifier_to_output_operators_algorithms_5_and_6),

        cmocka_unit_test(test_psg_chip_sets_attenuation),
        cmocka_unit_test(test_psg_chip_sets_frequency),
        cmocka_unit_test(test_psg_sets_busy_indicators),

        comm_test(test_comm_reads_when_ready),
        comm_test(test_comm_writes_when_ready),
        comm_test(test_comm_idle_count_is_correct),
        comm_test(test_comm_busy_count_is_correct),
        comm_test(test_comm_clamps_idle_count),
        comm_test(test_comm_clamps_busy_count),

        sonic_test(test_sonic_init), sonic_test(test_sonic_goes_into_idle_mode),
        sonic_test(test_sonic_single_vsync_does_nothing),
        sonic_test(test_sonic_runs), sonic_test(test_sonic_walks),
        sonic_test(test_sonic_spins)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
