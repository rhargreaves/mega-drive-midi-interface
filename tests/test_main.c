#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <test_interface.c>
#include <test_midi.c>
#include <test_synth.c>

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_interface_initialises_synth),
        cmocka_unit_test(test_interface_tick_passes_message_to_midi_processor),
        cmocka_unit_test(test_midi_triggers_synth_note_on),
        cmocka_unit_test(test_midi_triggers_synth_note_off),
        cmocka_unit_test(test_synth_init_sets_initial_registers),
        cmocka_unit_test(test_synth_sets_note_on_fm_reg),
        cmocka_unit_test(test_synth_sets_note_off_fm_reg)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
