#include "cmocka_inc.h"
#include "scheduler.h"

extern void __real_scheduler_init(void);
extern void __real_scheduler_tick(void);
extern void __real_scheduler_addTickHandler(HandlerFunc* onTick);
extern void __real_scheduler_addFrameHandler(HandlerFunc* onFrame);

static void dummy_frame_handler()
{
    function_called();
}

static void dummy_frame_handler_2()
{
    function_called();
}

static void dummy_tick_handler()
{
    function_called();
}

static void dummy_tick_handler_2()
{
    function_called();
}

static int test_scheduler_setup(UNUSED void** state)
{
    __real_scheduler_init();

    return 0;
}

static void test_scheduler_nothing_called_on_vsync(UNUSED void** state)
{
    scheduler_vsync();
}

static void test_scheduler_processes_frame_events_once_after_vsync(
    UNUSED void** state)
{
    __real_scheduler_addFrameHandler(*dummy_frame_handler);
    __real_scheduler_addTickHandler(*dummy_tick_handler);

    expect_function_call(dummy_tick_handler);
    __real_scheduler_tick();

    scheduler_vsync();

    expect_function_call(dummy_tick_handler);
    expect_function_call(dummy_frame_handler);
    __real_scheduler_tick();
}

static void test_scheduler_registered_frame_handler_called_on_vsync(
    UNUSED void** state)
{
    scheduler_vsync();

    __real_scheduler_addFrameHandler(*dummy_frame_handler);

    expect_function_call(dummy_frame_handler);
    __real_scheduler_tick();
}

static void test_scheduler_multiple_registered_frame_handlers_called_on_vsync(
    UNUSED void** state)
{
    scheduler_vsync();

    __real_scheduler_addFrameHandler(*dummy_frame_handler);
    __real_scheduler_addFrameHandler(*dummy_frame_handler_2);

    expect_function_call(dummy_frame_handler);
    expect_function_call(dummy_frame_handler_2);
    __real_scheduler_tick();
}

static void test_scheduler_registered_tick_handler_called(UNUSED void** state)
{
    __real_scheduler_addTickHandler(*dummy_tick_handler);

    expect_function_call(dummy_tick_handler);
    __real_scheduler_tick();
}

static void test_scheduler_multiple_registered_tick_handlers_called(
    UNUSED void** state)
{
    scheduler_vsync();

    __real_scheduler_addFrameHandler(*dummy_tick_handler);
    __real_scheduler_addFrameHandler(*dummy_tick_handler_2);

    expect_function_call(dummy_tick_handler);
    expect_function_call(dummy_tick_handler_2);
    __real_scheduler_tick();
}

static void test_scheduler_errors_if_too_many_frame_handlers_are_registered(
    UNUSED void** state)
{
    for (u16 i = 0; i < 6; i++) {
        __real_scheduler_addFrameHandler(*dummy_frame_handler);
    }

    expect_any(__wrap_SYS_die, err);
    __real_scheduler_addFrameHandler(*dummy_frame_handler);
}

static void test_scheduler_errors_if_too_many_tick_handlers_are_registered(
    UNUSED void** state)
{
    for (u16 i = 0; i < 3; i++) {
        __real_scheduler_addTickHandler(*dummy_tick_handler);
    }

    expect_any(__wrap_SYS_die, err);
    __real_scheduler_addTickHandler(*dummy_tick_handler);
}
