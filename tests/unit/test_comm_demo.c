#include "cmocka_inc.h"
#include "comm_demo.h"
#include <joy.h>

extern void __real_comm_demo_init(void);

static int test_comm_demo_setup(UNUSED void** state)
{
    __real_comm_demo_init();
    return 0;
}

static void test_comm_demo_is_ready_if_button_a_pressed(UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, BUTTON_A);

    u8 read = __real_comm_demo_read_ready();

    assert_int_equal(read, true);
}

static void test_comm_demo_is_not_ready_if_no_button_pressed(
    UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, 0);

    u8 read = __real_comm_demo_read_ready();

    assert_int_equal(read, false);
}
