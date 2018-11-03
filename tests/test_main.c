#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <foo.h>
#include <interface.h>

static void interface_tick_does_nothing(void** state)
{
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
        cmocka_unit_test(interface_tick_does_nothing),
        cmocka_unit_test(adds_two_integers),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
