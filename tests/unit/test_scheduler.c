#include "cmocka_inc.h"

#include "scheduler.h"

extern void __real_scheduler_init(void);
extern void __real_scheduler_tick(void);

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
    expect_function_call(__wrap_comm_megawifi_tick);
    expect_function_call(__wrap_midi_receiver_read_if_comm_ready);
    __real_scheduler_tick();

    scheduler_vsync();

    expect_function_call(__wrap_comm_megawifi_tick);
    expect_function_call(__wrap_midi_receiver_read_if_comm_ready);
    expect_function_call(__wrap_midi_psg_tick);
    expect_function_call(__wrap_ui_update);
    __real_scheduler_tick();
}

static void test_scheduler_tick_runs_midi_receiver(UNUSED void** state)
{
    expect_function_call(__wrap_comm_megawifi_tick);
    expect_function_call(__wrap_midi_receiver_read_if_comm_ready);

    __real_scheduler_tick();
}
