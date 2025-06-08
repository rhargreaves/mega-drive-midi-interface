#include "test_comm_demo.h"
#include "comm/comm_demo.h"
#include "joy.h"
#include "mocks/mock_comm.h"
#include "midi.h"

#define DEFAULT_PITCH 69

static void assert_read(u8 data);
static void assert_note_on(u8 pitch, u8 velocity);
static void assert_note_off(u8 pitch);
static void assert_cc(u8 cc, u8 value);
static void assert_program_change(u8 program);

int test_comm_demo_setup(UNUSED void** state)
{
    expect_any(__wrap_scheduler_addFrameHandler, onFrame);
    __real_comm_demo_init();
    return 0;
}

void test_comm_demo_is_ready_if_button_a_pressed(UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, BUTTON_A);

    u8 read = __real_comm_demo_read_ready();

    assert_int_equal(read, true);
}

void test_comm_demo_is_not_ready_if_no_button_pressed(UNUSED void** state)
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

static void vsync_call_multiple(u16 buttonPressed, int times)
{
    for (int i = 0; i < times; i++) {
        expect_function_call(__wrap_JOY_update);
        will_return(__wrap_JOY_readJoypad, buttonPressed);
        __real_comm_demo_vsync();
    }
}

static void assert_note_played_and_stopped(u8 pitch, u8 program)
{
    assert_cc(CC_SHOW_PARAMETERS_ON_UI, 0x7F);
    assert_program_change(program);
    assert_note_on(pitch, 127);

    for (int i = 0; i < 50; i++) {
        assert_int_equal(__real_comm_demo_read_ready(), false);
        vsync_call(0);
    }

    assert_note_off(pitch);

    for (int i = 0; i < 2; i++) {
        assert_int_equal(__real_comm_demo_read_ready(), false);
        vsync_call(0);
    }
}

void test_comm_demo_plays_note(UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, BUTTON_A);
    __real_comm_demo_read_ready();

    while (__real_comm_demo_read_ready()) {
        __real_comm_demo_read();
    }

    vsync_call(BUTTON_A);
    assert_note_off(DEFAULT_PITCH);
    assert_note_on(DEFAULT_PITCH, 127);
}

void test_comm_demo_increases_pitch(UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, BUTTON_A);
    __real_comm_demo_read_ready();

    while (__real_comm_demo_read_ready()) {
        __real_comm_demo_read();
    }

    vsync_call(BUTTON_UP);
    assert_note_off(DEFAULT_PITCH);
    assert_note_on(DEFAULT_PITCH + 1, 127);
}

void test_comm_demo_decreases_pitch(UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, BUTTON_A);
    __real_comm_demo_read_ready();

    while (__real_comm_demo_read_ready()) {
        __real_comm_demo_read();
    }

    vsync_call(BUTTON_DOWN);
    assert_note_off(DEFAULT_PITCH);
    assert_note_on(DEFAULT_PITCH - 1, 127);
}

void test_comm_demo_increases_program(UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, BUTTON_A);
    __real_comm_demo_read_ready();

    while (__real_comm_demo_read_ready()) {
        __real_comm_demo_read();
    }

    vsync_call(BUTTON_RIGHT);
    assert_note_off(DEFAULT_PITCH);
    assert_program_change(1);
    assert_note_on(DEFAULT_PITCH, 127);
}

void test_comm_demo_decreases_program(UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, BUTTON_A);
    __real_comm_demo_read_ready();

    while (__real_comm_demo_read_ready()) {
        __real_comm_demo_read();
    }

    vsync_call(BUTTON_RIGHT);
    assert_note_off(DEFAULT_PITCH);
    assert_program_change(1);
    assert_note_on(DEFAULT_PITCH, 127);

    vsync_call(BUTTON_LEFT);
    assert_note_off(DEFAULT_PITCH);
    assert_program_change(0);
    assert_note_on(DEFAULT_PITCH, 127);
}

void test_comm_demo_button_repeat_with_initial_delay(UNUSED void** state)
{
    will_return(__wrap_JOY_readJoypad, BUTTON_A);
    __real_comm_demo_read_ready();

    while (__real_comm_demo_read_ready()) {
        __real_comm_demo_read();
    }

    vsync_call(BUTTON_UP);
    assert_note_off(DEFAULT_PITCH);
    assert_note_on(DEFAULT_PITCH + 1, 127);

    vsync_call_multiple(BUTTON_UP, 38);
    assert_int_equal(__real_comm_demo_read_ready(), false);

    vsync_call(BUTTON_UP);
    assert_note_off(DEFAULT_PITCH + 1);
    assert_note_on(DEFAULT_PITCH + 2, 127);

    vsync_call_multiple(BUTTON_UP, 9);
    assert_int_equal(__real_comm_demo_read_ready(), false);

    vsync_call(BUTTON_UP);
    assert_note_off(DEFAULT_PITCH + 2);
    assert_note_on(DEFAULT_PITCH + 3, 127);
}

static void assert_read(u8 data)
{
    assert_int_equal(__real_comm_demo_read_ready(), true);
    assert_int_equal(__real_comm_demo_read(), data);
}

static void assert_note_on(u8 pitch, u8 velocity)
{
    assert_read(0x90);
    assert_read(pitch);
    assert_read(velocity);
}

static void assert_note_off(u8 pitch)
{
    assert_read(0x80);
    assert_read(pitch);
    assert_read(0);
}

static void assert_cc(u8 cc, u8 value)
{
    assert_read(0xB0);
    assert_read(cc);
    assert_read(value);
}

static void assert_program_change(u8 program)
{
    assert_read(0xC0);
    assert_read(program);
}