#include "cmocka_inc.h"
#include "mocks/mock_scheduler.h"

void __wrap_scheduler_tick(void)
{
    function_called();
}

void __wrap_scheduler_addTickHandler(HandlerFunc* onTick)
{
    check_expected(onTick);
}

void __wrap_scheduler_addFrameHandler(HandlerFunc* onFrame)
{
    check_expected(onFrame);
}
