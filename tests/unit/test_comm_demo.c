#include "cmocka_inc.h"
#include "comm_demo.h"

extern void __real_comm_demo_init(void);

static int test_comm_demo_setup(UNUSED void** state)
{
    __real_comm_demo_init();
    return 0;
}

static void test_comm_demo_plays_note_repeatedly(UNUSED void** state)
{
    // will_return(__wrap_comm_everdrive_read_ready, 0);
    // will_return(__wrap_comm_everdrive_pro_read_ready, 0);
    // will_return(__wrap_comm_serial_read_ready, 1);
    // will_return(__wrap_comm_serial_read, 50);

    // u8 read = __real_comm_demo_read_ready();

    // assert_int_equal(read, 50);
}
