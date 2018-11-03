#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <comm.h>
#include <foo.h>
#include <interface.h>
#include <midi.h>

u8 __wrap_comm_read(void)
{
    return mock_type(u8);
}

void __wrap_midi_process(u16 message)
{
    check_expected(message);
}

static void interface_tick_passes_message_to_midi_processor(void** state)
{
    u8 expected_message = 5;

    will_return(__wrap_comm_read, expected_message);

    expect_value(__wrap_midi_process, message, expected_message);

    interface_tick();
}

static void adds_two_integers(void** state)
{
    int result = add(2, 2);
    assert_int_equal(4, result);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(interface_tick_passes_message_to_midi_processor),
        cmocka_unit_test(adds_two_integers),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
