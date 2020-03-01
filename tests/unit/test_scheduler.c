#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "unused.h"
#include <cmocka.h>
#include <stdio.h>

void __real_scheduler_init(void);

static int test_scheduler_setup(void** state)
{
    __real_scheduler_init();

    return 0;
}

static void test_scheduler_does_frame_event_after_vsync(UNUSED void** state)
{
}
