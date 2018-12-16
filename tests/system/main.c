#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "test_e2e.c"
#include <cmocka.h>

int main(void)
{
    const struct CMUnitTest tests[]
        = { cmocka_unit_test(test_midi_note_on_event_sent_to_ym2612),
        cmocka_unit_test(test_midi_pitch_bend_sent_to_ym2612) };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
