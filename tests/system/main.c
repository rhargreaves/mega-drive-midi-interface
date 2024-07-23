#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "test_e2e.c"
#include <cmocka.h>

#define e2e_test(test) cmocka_unit_test_setup(test, test_e2e_setup)

int main(void)
{
    const struct CMUnitTest tests[] = {
        // clang-format off
        e2e_test(test_midi_note_on_event_sent_to_ym2612),
        e2e_test(test_polyphonic_midi_sent_to_separate_ym2612_channels),
        e2e_test(test_psg_not_audible_if_midi_channel_volume_set_and_there_is_no_note_on_event),
        e2e_test(test_psg_audible_if_note_on_event_triggered),
        e2e_test(test_general_midi_reset_sysex_stops_all_notes),
        e2e_test(test_remap_midi_channel_1_to_psg_channel_1),
        e2e_test(test_set_device_for_midi_channel_1_to_psg),
        e2e_test(test_pong_received_after_ping_sent),
        e2e_test(test_loads_psg_envelope),
        e2e_test(test_enables_ch3_special_mode),
        e2e_test(test_sets_separate_ch3_operator_frequencies),
        e2e_test(test_pitch_bends_ch3_special_mode_operators),
        e2e_test(test_write_directly_to_ym2612_regs_via_sysex),
        e2e_test(test_plays_pcm_sample),
        e2e_test(test_midi_last_note_played_priority_respected_on_fm),
        e2e_test(test_midi_last_note_played_cleared_when_released_on_fm),
        e2e_test(test_midi_last_note_played_remembers_velocity_on_fm),
        e2e_test(test_midi_changing_program_retains_volume),
        e2e_test(test_midi_changing_program_retains_pan),
        e2e_test(test_midi_portamento_glides_note)
        // clang-format on
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
