#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "comm_megawifi.h"
#include "unused.h"
#include <cmocka.h>

extern void __real_comm_megawifi_init(void);

static int test_comm_megawifi_setup(UNUSED void** state)
{
    __real_comm_megawifi_init();

    return 0;
}

static void test_comm_megawifi_initialises(UNUSED void** state)
{
    __real_comm_megawifi_init();

    assert_int_equal(0, 0);
}
