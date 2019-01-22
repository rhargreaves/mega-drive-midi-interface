#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "test_comm.c"
#include "test_interface.c"
#include "test_midi.c"
#include "test_psg_chip.c"
#include "test_synth.c"
#include <cmocka.h>

int main(void)
{
    const struct CMUnitTest tests[] = { cmocka_unit_test(
                                            test_interface_initialises_synth),
        cmocka_unit_test(test_interface_tick_passes_note_on_to_midi_processor),
        cmocka_unit_test(test_interface_tick_passes_note_off_to_midi_processor),
        cmocka_unit_test(test_interface_does_nothing_for_control_change),
        cmocka_unit_test(test_interface_sets_unknown_event_for_unknown_status),
        cmocka_unit_test(
            test_interface_sets_unknown_event_for_unknown_system_message),
        cmocka_unit_test(test_interface_sets_CC),
        cmocka_unit_test(test_interface_sets_pitch_bend),
        cmocka_unit_test(test_interface_increments_midi_clock),
        cmocka_unit_test(test_interface_starts_midi),
        cmocka_unit_test(test_interface_swallows_midi_stop),

        cmocka_unit_test(test_midi_triggers_synth_note_on),
        cmocka_unit_test(test_midi_triggers_synth_note_on_2),
        cmocka_unit_test(test_midi_triggers_synth_note_off),
        cmocka_unit_test(test_midi_triggers_psg_note_on),
        cmocka_unit_test(test_midi_triggers_psg_note_off),
        cmocka_unit_test(test_midi_channel_volume_sets_total_level),
        cmocka_unit_test(test_midi_pan_sets_synth_stereo_mode_right),
        cmocka_unit_test(test_midi_pan_sets_synth_stereo_mode_left),
        cmocka_unit_test(test_midi_pan_sets_synth_stereo_mode_centre),
        cmocka_unit_test(test_midi_channel_volume_sets_psg_attenuation),
        cmocka_unit_test(test_midi_channel_volume_sets_psg_attenuation_2),
        cmocka_unit_test(test_midi_ignores_channels_above_10),
        cmocka_unit_test_setup(
            test_midi_sets_synth_pitch_bend, test_midi_setup),
        cmocka_unit_test_setup(test_midi_sets_psg_pitch_bend, test_midi_setup),
        cmocka_unit_test(test_midi_polyphonic_mode_returns_state),
        cmocka_unit_test_setup(
            test_midi_polyphonic_mode_uses_multiple_fm_channels,
            test_midi_setup),
        cmocka_unit_test_setup(
            test_midi_polyphonic_mode_note_off_silences_all_matching_pitch,
            test_midi_setup),
        cmocka_unit_test(test_midi_sets_all_notes_off),
        cmocka_unit_test_setup_teardown(
            test_midi_sets_all_notes_off_in_polyphonic_mode, test_midi_setup,
            test_midi_setup),
        cmocka_unit_test(test_midi_sets_fm_algorithm),
        cmocka_unit_test(test_midi_sets_fm_feedback),
        cmocka_unit_test(test_midi_sets_operator_total_level),
        cmocka_unit_test(test_midi_sets_operator_multiple),
        cmocka_unit_test(test_midi_sets_operator_detune),
        cmocka_unit_test(test_midi_sets_operator_rate_scaling),
        cmocka_unit_test(test_midi_sets_operator_attack_rate),
        cmocka_unit_test(test_midi_sets_operator_first_decay_rate),
        cmocka_unit_test(test_midi_sets_operator_second_decay_rate),
        cmocka_unit_test(test_midi_sets_operator_secondary_amplitude),
        cmocka_unit_test(test_midi_sets_operator_amplitude_modulation),
        cmocka_unit_test(test_midi_sets_operator_release_rate),
        cmocka_unit_test(test_midi_sets_global_LFO_enable),
        cmocka_unit_test(test_midi_sets_global_LFO_frequency),
        cmocka_unit_test(test_midi_sets_channel_AMS),
        cmocka_unit_test(test_midi_sets_channel_FMS),
        cmocka_unit_test(test_midi_sets_polyphonic_mode),
        cmocka_unit_test(test_midi_unsets_polyphonic_mode),
        cmocka_unit_test(test_midi_sets_unknown_CC),
        cmocka_unit_test(
            test_midi_polyphonic_mode_sends_CCs_to_all_FM_channels),
        cmocka_unit_test_setup_teardown(
            test_midi_set_overflow_flag_on_polyphony_breach, test_midi_setup,
            test_midi_setup),
        cmocka_unit_test_setup_teardown(
            test_midi_clears_overflow_flag, test_midi_setup, test_midi_setup),
        cmocka_unit_test_setup_teardown(
            test_midi_increments_beat_every_6th_clock, test_midi_setup,
            test_midi_setup),
        cmocka_unit_test_setup_teardown(
            test_midi_start_resets_clock, test_midi_setup, test_midi_setup),

        cmocka_unit_test(test_synth_init_sets_initial_registers),
        cmocka_unit_test(test_synth_sets_note_on_fm_reg_chan_0_to_2),
        cmocka_unit_test(test_synth_sets_note_on_fm_reg_chan_3_to_5),
        cmocka_unit_test(test_synth_sets_note_off_fm_reg_chan_0_to_2),
        cmocka_unit_test(test_synth_sets_note_off_fm_reg_chan_3_to_5),
        cmocka_unit_test(test_synth_sets_octave_and_freq_reg_chan),
        cmocka_unit_test(
            test_synth_sets_total_level_reg_chan_for_algorithms_0_to_3),
        cmocka_unit_test(test_synth_sets_total_level_reg_chan_for_algorithm_4),
        cmocka_unit_test(
            test_synth_sets_total_level_reg_chan_for_algorithms_5_and_6),
        cmocka_unit_test(test_synth_sets_total_level_reg_chan_for_algorithm_7),
        cmocka_unit_test(test_synth_sets_stereo_ams_and_freq),
        cmocka_unit_test_setup(test_synth_sets_algorithm, test_synth_setup),
        cmocka_unit_test_setup(test_synth_sets_feedback, test_synth_setup),
        cmocka_unit_test(test_synth_sets_operator_total_level),
        cmocka_unit_test_setup(
            test_synth_sets_feedback_and_algorithm, test_synth_setup),
        cmocka_unit_test_setup(
            test_synth_sets_operator_multiple_and_detune, test_synth_setup),
        cmocka_unit_test_setup(
            test_synth_sets_operator_attack_rate_and_rate_scaling,
            test_synth_setup),
        cmocka_unit_test(test_synth_sets_operator_second_decay_rate),
        cmocka_unit_test_setup(
            test_synth_sets_operator_release_rate_and_secondary_amplitude,
            test_synth_setup),
        cmocka_unit_test_setup(
            test_synth_sets_operator_amplitude_modulation_and_first_decay_rate,
            test_synth_setup),
        cmocka_unit_test(test_synth_sets_global_LFO_enable_and_frequency),
        cmocka_unit_test_setup(
            test_synth_sets_busy_indicators, test_synth_setup),

        cmocka_unit_test(test_psg_chip_sets_note_on_psg),
        cmocka_unit_test(test_psg_chip_sets_note_off_psg),
        cmocka_unit_test(test_psg_chip_sets_attenuation),
        cmocka_unit_test(test_psg_chip_sets_frequency),
        cmocka_unit_test(test_psg_chip_sets_note_on_psg_with_attenuation),
        cmocka_unit_test(test_psg_sets_busy_indicators),

        cmocka_unit_test_setup(test_comm_reads_when_ready, test_comm_setup),
        cmocka_unit_test_setup(
            test_comm_idle_count_is_correct, test_comm_setup),
        cmocka_unit_test_setup(
            test_comm_busy_count_is_correct, test_comm_setup),
        cmocka_unit_test_setup(test_comm_clamps_idle_count, test_comm_setup),
        cmocka_unit_test_setup(test_comm_clamps_busy_count, test_comm_setup) };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
