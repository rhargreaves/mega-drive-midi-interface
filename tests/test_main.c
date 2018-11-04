#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <comm.h>
#include <interface.h>
#include <midi.h>
#include <wraps.h>

#include <test_interface.c>

static void midi_triggers_synth_note_on(void** state)
{
    expect_value(__wrap_synth_noteOn, channel, 0);

    Message noteOn = { 0b10010000, 0x40, 127 };

    __real_midi_process(&noteOn);
}

static void synth_init_sets_initial_registers(void** state)
{
    u16 count = 39;
    expect_any_count(__wrap_fm_writeReg, part, count);
    expect_any_count(__wrap_fm_writeReg, reg, count);
    expect_any_count(__wrap_fm_writeReg, data, count);

    __real_synth_init();
}

static void synth_writes_fm_reg(void** state)
{
    expect_value(__wrap_fm_writeReg, part, 0);
    expect_value(__wrap_fm_writeReg, reg, 0x28);
    expect_value(__wrap_fm_writeReg, data, 0xF0);

    __real_synth_noteOn(0);
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
