#include "cmocka_inc.h"
#include "test_applemidi.c"
#include "test_comm.c"
#include "test_comm_megawifi.c"
#include "test_comm_demo.c"
#include "test_log.c"
#include "test_midi.h"
#include "test_midi_dynamic.c"
#include "test_midi_fm.c"
#include "test_midi_polyphony.c"
#include "test_midi_psg.c"
#include "test_midi_receiver.c"
#include "test_midi_sysex.c"
#include "test_scheduler.c"
#include "test_synth.c"
#include "test_vstring.c"
#include "test_buffer.c"

#define midi_test(test) cmocka_unit_test_setup(test, test_midi_setup)
#define dynamic_midi_test(test)                                                \
    cmocka_unit_test_setup(test, test_dynamic_midi_setup)
#define synth_test(test) cmocka_unit_test_setup(test, test_synth_setup)
#define comm_test(test) cmocka_unit_test_setup(test, test_comm_setup)
#define comm_megawifi_test(test)                                               \
    cmocka_unit_test_setup(test, test_comm_megawifi_setup)
#define comm_demo_test(test) cmocka_unit_test_setup(test, test_comm_demo_setup)
#define log_test(test) cmocka_unit_test_setup(test, test_log_setup)
#define scheduler_test(test) cmocka_unit_test_setup(test, test_scheduler_setup)
#define applemidi_test(test) cmocka_unit_test_setup(test, test_applemidi_setup)
#define buffer_test(test) cmocka_unit_test_setup(test, test_buffer_setup)
#define midi_receiver_test(test)                                               \
    cmocka_unit_test_setup(test, test_midi_receiver_setup)

int main(void)
{
    const struct CMUnitTest tests[] = {
        midi_receiver_test(
            test_midi_receiver_read_passes_note_on_to_midi_processor),
        midi_receiver_test(
            test_midi_receiver_read_passes_note_off_to_midi_processor),
        midi_receiver_test(test_midi_receiver_does_nothing_for_control_change),
        midi_receiver_test(
            test_midi_receiver_sets_unknown_event_for_unknown_status),
        midi_receiver_test(
            test_midi_receiver_sets_unknown_event_for_unknown_system_message),
        midi_receiver_test(test_midi_receiver_sets_CC),
        midi_receiver_test(test_midi_receiver_sets_pitch_bend),
        midi_receiver_test(test_midi_receiver_does_nothing_on_midi_clock),
        midi_receiver_test(test_midi_receiver_does_nothing_on_midi_start_midi),
        midi_receiver_test(test_midi_receiver_swallows_midi_stop),
        midi_receiver_test(test_midi_receiver_swallows_midi_continue),
        midi_receiver_test(test_midi_receiver_does_nothing_on_midi_position),
        midi_receiver_test(test_midi_receiver_sets_midi_program),
        midi_receiver_test(test_midi_receiver_sends_sysex_to_midi_layer),
        midi_receiver_test(test_midi_receiver_handles_sysex_limits),
        midi_receiver_test(test_midi_receiver_sends_midi_reset),

        midi_test(test_midi_triggers_synth_note_on),
        midi_test(test_midi_triggers_synth_note_on_with_velocity),
        midi_test(
            test_midi_triggers_synth_note_on_with_velocity_and_channel_volume),
        midi_test(test_midi_triggers_synth_note_on_boundary_values),
        midi_test(test_midi_does_not_trigger_synth_note_on_out_of_bound_values),
        midi_test(test_midi_triggers_synth_note_on_2),
        midi_test(test_midi_triggers_synth_note_off),
        midi_test(
            test_midi_triggers_synth_note_off_when_note_on_has_zero_velocity),
        midi_test(test_midi_triggers_psg_note_on),
        midi_test(test_midi_uses_PAL_tones_if_system_is_in_that_region),
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
        midi_test(test_midi_plays_psg_envelope),
        midi_test(test_midi_plays_advanced_psg_envelope),
        midi_test(test_midi_loops_psg_envelope),
        midi_test(test_midi_psg_envelope_with_only_end_flag_is_silent),
        midi_test(
            test_midi_psg_envelope_with_loop_end_continues_playing_after_note_off),
        midi_test(
            test_midi_psg_envelope_with_loop_end_resets_release_note_after_note_silenced),
        midi_test(test_midi_shifts_semitone_in_psg_envelope),
        midi_test(test_midi_pitch_shift_handles_upper_limit_psg_envelope),
        midi_test(test_midi_pitch_shift_handles_lower_limit_psg_envelope),
        midi_test(test_midi_psg_sets_busy_indicators),
        midi_test(test_midi_channel_volume_sets_volume),
        midi_test(test_midi_pan_sets_synth_stereo_mode_right),
        midi_test(test_midi_pan_sets_synth_stereo_mode_left),
        midi_test(test_midi_pan_sets_synth_stereo_mode_centre),
        midi_test(test_midi_channel_volume_sets_psg_attenuation),
        midi_test(test_midi_channel_volume_sets_psg_attenuation_2),
        midi_test(test_midi_sets_synth_pitch_bend),
        midi_test(test_midi_sets_psg_pitch_bend_down),
        midi_test(test_midi_sets_psg_pitch_bend_up),
        midi_test(test_midi_psg_pitch_bend_persists_after_tick),
        midi_test(test_midi_loads_psg_envelope),
        midi_test(test_midi_polyphonic_mode_returns_state),
        midi_test(test_midi_polyphonic_mode_uses_multiple_fm_channels),
        midi_test(
            test_midi_polyphonic_mode_note_off_silences_all_matching_pitch),
        midi_test(test_midi_sets_all_notes_off),
        midi_test(test_midi_sets_all_sound_off),
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
        midi_test(test_midi_ignores_sustain_pedal_cc),
        midi_test(test_midi_ignores_sysex_nrpn_ccs),
        midi_test(test_midi_polyphonic_mode_sends_CCs_to_all_FM_channels),
        midi_test(test_midi_set_overflow_flag_on_polyphony_breach),
        midi_test(test_midi_sets_fm_preset),
        midi_test(test_midi_sysex_general_midi_reset_resets_synth_volume),
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
        midi_test(test_midi_enables_fm_special_mode),
        midi_test(test_midi_disables_fm_special_mode),
        midi_test(test_midi_sets_pitch_of_special_mode_ch3_operator),

        midi_test(test_midi_sets_genmdm_stereo_mode),
        midi_test(test_midi_sysex_enables_dynamic_channel_mode),
        midi_test(test_midi_sysex_sets_mapping_mode_to_auto),
        midi_test(test_midi_sysex_disables_fm_parameter_CCs),
        midi_test(test_midi_sysex_loads_psg_envelope),
        midi_test(test_midi_sysex_inverts_total_level_values),
        midi_test(test_midi_sysex_sets_original_total_level_values),
        midi_test(
            test_midi_sets_all_channel_mappings_when_setting_polyphonic_mode),
        midi_test(test_midi_shows_fm_parameter_ui),
        midi_test(test_midi_hides_fm_parameter_ui),
        midi_test(test_midi_reset_reinitialises_module),

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
        synth_test(test_synth_does_not_reset_operator_level_if_equal),
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
        synth_test(test_synth_does_not_apply_volume_if_equal),
        synth_test(
            test_synth_applies_volume_modifier_to_output_operators_algorithms_0_to_3),
        synth_test(
            test_synth_applies_volume_modifier_to_output_operators_algorithm_4),
        synth_test(
            test_synth_applies_volume_modifier_to_output_operators_algorithms_5_and_6),
        synth_test(test_synth_exposes_fm_channel_parameters),
        synth_test(test_synth_exposes_global_parameters),
        synth_test(test_synth_calls_callback_when_parameter_changes),
        synth_test(test_synth_calls_callback_when_lfo_freq_changes),
        synth_test(test_synth_calls_callback_when_lfo_enable_changes),
        synth_test(test_synth_enables_ch3_special_mode),
        synth_test(test_synth_disables_ch3_special_mode),
        synth_test(test_synth_sets_ch3_special_mode_operator_pitches),

        comm_test(test_comm_reads_from_serial_when_ready),
        comm_test(test_comm_reads_when_ready),
        comm_test(test_comm_writes_when_ready),
        comm_test(test_comm_idle_count_is_correct),
        comm_test(test_comm_busy_count_is_correct),
        comm_test(test_comm_clamps_idle_count),
        comm_test(test_comm_clamps_busy_count),

        comm_demo_test(test_comm_demo_is_ready_if_button_a_pressed),
        comm_demo_test(test_comm_demo_is_not_ready_if_no_button_pressed),
        comm_demo_test(test_comm_demo_plays_note),
        comm_demo_test(test_comm_demo_increases_pitch),
        comm_demo_test(test_comm_demo_decreases_pitch),
        comm_demo_test(test_comm_demo_increases_program),
        comm_demo_test(test_comm_demo_decreases_program),

        comm_megawifi_test(test_comm_megawifi_initialises),
        comm_megawifi_test(test_comm_megawifi_reads_midi_message),
        comm_megawifi_test(test_comm_megawifi_logs_if_buffer_full),

        dynamic_midi_test(test_midi_dynamic_uses_all_channels),
        dynamic_midi_test(test_midi_routing_switches_to_dynamic_on_gm_reset),
        dynamic_midi_test(
            test_midi_dynamic_tries_to_reuse_original_midi_channel_if_available),
        dynamic_midi_test(test_midi_reports_dynamic_mode_enabled),
        dynamic_midi_test(test_midi_reports_dynamic_mode_disabled),
        dynamic_midi_test(test_midi_exposes_dynamic_mode_mappings),
        dynamic_midi_test(test_midi_dynamic_enables_percussive_mode_if_needed),
        dynamic_midi_test(test_midi_sets_presets_on_dynamic_channels),
        dynamic_midi_test(
            test_midi_dynamic_does_not_send_percussion_to_psg_channels),
        dynamic_midi_test(test_midi_sysex_resets_dynamic_mode_state),
        dynamic_midi_test(
            test_midi_dynamic_sends_note_off_to_channel_playing_same_pitch),
        dynamic_midi_test(test_midi_dynamic_limits_percussion_notes),
        dynamic_midi_test(test_midi_dynamic_maintains_volume_on_remapping),
        dynamic_midi_test(test_midi_dynamic_sets_volume_on_playing_notes),
        dynamic_midi_test(test_midi_dynamic_maintains_pan_on_remapping),
        dynamic_midi_test(test_midi_dynamic_maintains_pitch_bend_on_remapping),
        dynamic_midi_test(test_midi_dynamic_resets_mappings_on_cc_121),
        dynamic_midi_test(test_midi_dynamic_all_notes_off_on_cc_123),
        dynamic_midi_test(test_midi_dynamic_sysex_remaps_midi_channel),
        dynamic_midi_test(
            test_midi_dynamic_sysex_removes_mapping_of_midi_channel),
        dynamic_midi_test(
            test_midi_dynamic_prefers_psg_for_square_wave_instruments),
        dynamic_midi_test(
            test_midi_dynamic_reuses_mapped_midi_channel_even_if_busy_if_sticking_to_device_type),
        dynamic_midi_test(
            test_midi_dynamic_sticks_to_assigned_device_type_for_midi_channels),
        dynamic_midi_test(
            test_midi_dynamic_sticks_to_assigned_psg_device_type_for_midi_channels),
        dynamic_midi_test(test_midi_assign_channel_to_psg_device),
        dynamic_midi_test(test_midi_assign_channel_to_fm_device_only),
        dynamic_midi_test(test_midi_assign_channel_to_psg_noise),

        log_test(test_log_info_writes_to_log_buffer),
        log_test(test_log_warn_writes_to_log_buffer),
        log_test(test_log_stores_two_logs),
        log_test(test_log_stores_multiple_logs_and_overwrites_older),
        log_test(
            test_log_returns_null_when_no_more_logs_are_available_to_be_dequeued),

        scheduler_test(test_scheduler_nothing_called_on_vsync),
        scheduler_test(test_scheduler_processes_frame_events_once_after_vsync),
        scheduler_test(test_scheduler_registered_frame_handler_called_on_vsync),
        scheduler_test(test_scheduler_registered_tick_handler_called),
        scheduler_test(
            test_scheduler_multiple_registered_frame_handlers_called_on_vsync),
        scheduler_test(test_scheduler_multiple_registered_tick_handlers_called),
        scheduler_test(
            test_scheduler_errors_if_too_many_frame_handlers_are_registered),
        scheduler_test(
            test_scheduler_errors_if_too_many_tick_handlers_are_registered),

        applemidi_test(
            test_applemidi_parses_rtpmidi_packet_with_single_midi_event),
        applemidi_test(
            test_applemidi_parses_rtpmidi_packet_with_single_midi_event_long_header),
        applemidi_test(
            test_applemidi_parses_rtpmidi_packet_with_single_2_byte_midi_event),
        applemidi_test(
            test_applemidi_parses_rtpmidi_packet_with_two_midi_events),
        applemidi_test(
            test_applemidi_parses_rtpmidi_packet_with_multiple_midi_events),
        applemidi_test(
            test_applemidi_parses_rtpmidi_packet_with_multiple_different_midi_events),
        applemidi_test(
            test_applemidi_parses_rtpmidi_packet_with_multiple_2_byte_midi_events),
        applemidi_test(test_applemidi_parses_rtpmidi_packet_with_sysex),
        applemidi_test(test_applemidi_parses_notes_sysex_cc_in_one_packet),
        applemidi_test(test_applemidi_ignores_middle_sysex_segments),
        applemidi_test(test_applemidi_processes_multiple_sysex_segments),
        applemidi_test(test_applemidi_processes_ccs),
        applemidi_test(test_applemidi_sets_last_sequence_number),
        applemidi_test(test_applemidi_sends_receiver_feedback),
        applemidi_test(
            test_applemidi_parses_rtpmidi_packet_with_sysex_ending_with_F0),
        applemidi_test(
            test_applemidi_parses_rtpmidi_packet_with_sysex_with_0xF7_at_end),
        applemidi_test(test_applemidi_does_not_read_beyond_length),
        applemidi_test(test_applemidi_parses_rtpmidi_packet_with_system_reset),

        cmocka_unit_test(test_vstring_handles_variable_argument_list_correctly),

        buffer_test(test_buffer_reads_and_writes_single_byte),
        buffer_test(test_buffer_reads_and_writes_circularly_over_capacity),
        buffer_test(test_buffer_available_returns_correct_value),
        buffer_test(test_buffer_available_returns_correct_value_when_empty),
        buffer_test(test_buffer_available_returns_correct_value_when_full),
        buffer_test(test_buffer_returns_cannot_write_if_full),
        buffer_test(test_buffer_returns_can_write_if_empty)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
