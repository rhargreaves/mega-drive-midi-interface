#include "cmocka_inc.h"
#include "comm_demo.h"
#include <joy.h>

#define DEFAULT_PITCH 69

extern void __real_comm_demo_init(void);

static int test_comm_demo_setup(UNUSED void** state)
{
    expect_any(__wrap_scheduler_addFrameHandler, onFrame);
    __real_comm_demo_init();
    return 0;
}

static void test_comm_demo_is_ready_if_button_a_pressed(UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, BUTTON_A);

    u8 read = __real_comm_demo_read_ready();

    assert_int_equal(read, true);
}

static void test_comm_demo_is_not_ready_if_no_button_pressed(UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, 0);

    u8 read = __real_comm_demo_read_ready();

    assert_int_equal(read, false);
}

static void vsync_call(u16 buttonPressed)
{
    expect_function_call(__wrap_JOY_update);
    will_return(__wrap_JOY_readJoypad, buttonPressed);
    __real_comm_demo_vsync();
}

static void assert_note_played_and_stopped(u8 pitch, u8 program)
{
    assert_int_equal(__real_comm_demo_read_ready(), true);
    assert_int_equal(__real_comm_demo_read(), 0xC0);

    assert_int_equal(__real_comm_demo_read_ready(), true);
    assert_int_equal(__real_comm_demo_read(), program);

    assert_int_equal(__real_comm_demo_read_ready(), true);
    assert_int_equal(__real_comm_demo_read(), 0x90);

    assert_int_equal(__real_comm_demo_read_ready(), true);
    assert_int_equal(__real_comm_demo_read(), pitch);

    assert_int_equal(__real_comm_demo_read_ready(), true);
    assert_int_equal(__real_comm_demo_read(), 127);

    for (int i = 0; i < 50; i++) {
        assert_int_equal(__real_comm_demo_read_ready(), false);
        vsync_call(0);
    }

    assert_int_equal(__real_comm_demo_read_ready(), true);
    assert_int_equal(__real_comm_demo_read(), 0x80);

    assert_int_equal(__real_comm_demo_read_ready(), true);
    assert_int_equal(__real_comm_demo_read(), pitch);

    assert_int_equal(__real_comm_demo_read_ready(), true);
    assert_int_equal(__real_comm_demo_read(), 127);

    for (int i = 0; i < 2; i++) {
        assert_int_equal(__real_comm_demo_read_ready(), false);
        vsync_call(0);
    }
}

static void test_comm_demo_plays_note(UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, BUTTON_A);

    for (int rep = 0; rep < 10; rep++) {
        assert_note_played_and_stopped(DEFAULT_PITCH, 0);
    }
}

static void test_comm_demo_increases_pitch(UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, BUTTON_A);
    assert_note_played_and_stopped(DEFAULT_PITCH, 0);

    vsync_call(BUTTON_UP);
    assert_note_played_and_stopped(DEFAULT_PITCH + 1, 0);
}

static void test_comm_demo_decreases_pitch(UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, BUTTON_A);
    assert_note_played_and_stopped(DEFAULT_PITCH, 0);

    vsync_call(BUTTON_DOWN);
    assert_note_played_and_stopped(DEFAULT_PITCH - 1, 0);
}

static void test_comm_demo_increases_program(UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, BUTTON_A);
    assert_note_played_and_stopped(DEFAULT_PITCH, 0);

    vsync_call(BUTTON_RIGHT);
    assert_note_played_and_stopped(DEFAULT_PITCH, 1);
}

static void test_comm_demo_decreases_program(UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, BUTTON_A);
    assert_note_played_and_stopped(DEFAULT_PITCH, 0);

    vsync_call(BUTTON_RIGHT);
    assert_note_played_and_stopped(DEFAULT_PITCH, 1);

    vsync_call(BUTTON_RIGHT);
    assert_note_played_and_stopped(DEFAULT_PITCH, 2);

    vsync_call(BUTTON_LEFT);
    assert_note_played_and_stopped(DEFAULT_PITCH, 1);
}
