#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "unused.h"
#include <cmocka.h>
#include <stdio.h>

#include "scheduler.h"

void __real_scheduler_init(void);

static int test_scheduler_setup(void** state)
{
    __real_scheduler_init();

    return 0;
}

static void test_scheduler_updates_ui_once_after_vsync(UNUSED void** state)
{
    expect_function_call(__wrap_ui_update);

    scheduler_vsync();
    scheduler_doEvents();
    scheduler_doEvents();
}
