#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <comm.h>
#include <interface.h>
#include <midi.h>
#include <wraps.h>

#include <test_interface.c>
#include <test_synth.c>

static void midi_triggers_synth_note_on(void** state)
{
    expect_value(__wrap_synth_noteOn, channel, 0);

    Message noteOn = { 0b10010000, 0x40, 127 };

    __real_midi_process(&noteOn);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(interface_initialises_synth),
        cmocka_unit_test(interface_tick_passes_message_to_midi_processor),
        cmocka_unit_test(midi_triggers_synth_note_on),
        cmocka_unit_test(synth_init_sets_initial_registers),
        cmocka_unit_test(synth_writes_fm_reg)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
