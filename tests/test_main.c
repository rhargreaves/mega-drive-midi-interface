#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <comm.h>
#include <interface.h>
#include <midi.h>

void __wrap_synth_init(void)
{
    function_called();
}

void __wrap_synth_noteOn(u8 channel)
{
    check_expected(channel);
}

u8 __wrap_comm_read(void)
{
    return mock_type(u8);
}

void __wrap_midi_process(Message* message)
{
    check_expected(message);
}

void __wrap_fm_writeReg(u16 part, u8 reg, u8 data)
{
    check_expected(part);
    check_expected(reg);
    check_expected(data);
}

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

static void interface_tick_passes_message_to_midi_processor(void** state)
{
    u8 expectedStatus = 1;
    u8 expectedData = 2;
    u8 expectedData2 = 3;
    Message expectedMessage = { expectedStatus, expectedData, expectedData2 };

    will_return(__wrap_comm_read, expectedStatus);
    will_return(__wrap_comm_read, expectedData);
    will_return(__wrap_comm_read, expectedData2);

    expect_memory(__wrap_midi_process, message, &expectedMessage, sizeof(Message));

    interface_tick();
}

static void interface_initialises_synth(void** state)
{
    expect_function_call(__wrap_synth_init);

    interface_init();
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
