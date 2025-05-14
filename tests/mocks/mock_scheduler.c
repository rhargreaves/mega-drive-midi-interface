#include "cmocka_inc.h"
#include "mocks/mock_scheduler.h"

void __wrap_scheduler_tick(void)
{
    function_called();
}

void __wrap_scheduler_addTickHandler(TickHandlerFunc* onTick)
{
    check_expected(onTick);
}

void __wrap_scheduler_addFrameHandler(FrameHandlerFunc* onFrame)
{
    check_expected(onFrame);
}

void __wrap_scheduler_yield(void)
{
    function_called();
}

void expect_scheduler_yield(void)
{
    expect_function_call(__wrap_scheduler_yield);
}
