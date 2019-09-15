#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "test_e2e.c"
#include <cmocka.h>

#define e2e_test(test) cmocka_unit_test_setup(test, test_e2e_setup)

int main(void)
{
    const struct CMUnitTest tests[] = {
        e2e_test(test_midi_note_on_event_sent_to_ym2612),
        e2e_test(test_midi_pitch_bend_sent_to_ym2612),
        e2e_test(test_polyphonic_midi_sent_to_separate_ym2612_channels),
        e2e_test(
            test_psg_not_audible_if_midi_channel_volume_set_and_there_is_no_note_on_event),
        e2e_test(test_psg_audible_if_note_on_event_triggered),
        e2e_test(test_general_midi_reset_sysex_stops_all_notes),
        e2e_test(test_remap_midi_channel_1_to_psg_channel_1)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
