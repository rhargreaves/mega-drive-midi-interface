#include "test_scheduler.h"
#include "cmocka_inc.h"
#include "scheduler.h"
#include "mocks/mock_scheduler.h"

static u16 dummy_frame_handler_delta = 0;

static void dummy_frame_handler(u16 delta)
{
    dummy_frame_handler_delta = delta;
    function_called();
}

static void dummy_frame_handler_2(u16 delta)
{
    function_called();
}

static void dummy_tick_handler(void)
{
    function_called();
}

static void dummy_tick_handler_2(void)
{
    function_called();
}

int test_scheduler_setup(UNUSED void** state)
{
    dummy_frame_handler_delta = 0;

    __real_scheduler_init();

    return 0;
}

void test_scheduler_nothing_called_on_vsync(UNUSED void** state)
{
    scheduler_vsync();
}

void test_scheduler_processes_frame_events_once_after_vsync(UNUSED void** state)
{
    __real_scheduler_addFrameHandler(dummy_frame_handler);
    __real_scheduler_addTickHandler(dummy_tick_handler);

    expect_function_call(dummy_tick_handler);
    __real_scheduler_tick();

    scheduler_vsync();

    expect_function_call(dummy_tick_handler);
    expect_function_call(dummy_frame_handler);
    __real_scheduler_tick();
}

void test_scheduler_registered_frame_handler_called_on_vsync(UNUSED void** state)
{
    scheduler_vsync();

    __real_scheduler_addFrameHandler(*dummy_frame_handler);

    expect_function_call(dummy_frame_handler);
    __real_scheduler_tick();
}

void test_scheduler_multiple_registered_frame_handlers_called_on_vsync(UNUSED void** state)
{
    scheduler_vsync();

    __real_scheduler_addFrameHandler(*dummy_frame_handler);
    __real_scheduler_addFrameHandler(*dummy_frame_handler_2);

    expect_function_call(dummy_frame_handler);
    expect_function_call(dummy_frame_handler_2);
    __real_scheduler_tick();
}

void test_scheduler_registered_tick_handler_called(UNUSED void** state)
{
    __real_scheduler_addTickHandler(*dummy_tick_handler);

    expect_function_call(dummy_tick_handler);
    __real_scheduler_tick();
}

void test_scheduler_multiple_registered_tick_handlers_called(UNUSED void** state)
{
    scheduler_vsync();

    __real_scheduler_addFrameHandler(dummy_frame_handler);
    __real_scheduler_addFrameHandler(dummy_frame_handler_2);

    expect_function_call(dummy_frame_handler);
    expect_function_call(dummy_frame_handler_2);
    __real_scheduler_tick();
}

void test_scheduler_errors_if_too_many_frame_handlers_are_registered(UNUSED void** state)
{
    for (u16 i = 0; i < 6; i++) {
        __real_scheduler_addFrameHandler(*dummy_frame_handler);
    }

    expect_any(__wrap_SYS_die, err);
    __real_scheduler_addFrameHandler(*dummy_frame_handler);
}

void test_scheduler_errors_if_too_many_tick_handlers_are_registered(UNUSED void** state)
{
    for (u16 i = 0; i < 3; i++) {
        __real_scheduler_addTickHandler(*dummy_tick_handler);
    }

    expect_any(__wrap_SYS_die, err);
    __real_scheduler_addTickHandler(*dummy_tick_handler);
}

void test_scheduler_yield_processes_pending_events(UNUSED void** state)
{
    __real_scheduler_addFrameHandler(dummy_frame_handler);
    __real_scheduler_addTickHandler(dummy_tick_handler);

    expect_function_call(dummy_tick_handler);
    __real_scheduler_yield();

    scheduler_vsync();

    expect_function_call(dummy_tick_handler);
    expect_function_call(dummy_frame_handler);
    __real_scheduler_yield();
}

void test_scheduler_passes_delta_to_frame_handlers(UNUSED void** state)
{
    __real_scheduler_addFrameHandler(dummy_frame_handler);

    scheduler_vsync();

    expect_function_call(dummy_frame_handler);
    __real_scheduler_tick();

    assert_int_equal(dummy_frame_handler_delta, 1);
}

void test_scheduler_passes_delta_given_multiple_vsyncs_to_frame_handlers(UNUSED void** state)
{
    __real_scheduler_addFrameHandler(dummy_frame_handler);

    scheduler_vsync();
    scheduler_vsync();

    expect_function_call(dummy_frame_handler);
    __real_scheduler_tick();

    assert_int_equal(dummy_frame_handler_delta, 2);
}