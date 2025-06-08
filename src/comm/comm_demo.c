#include "comm_demo.h"
#include "midi_fm.h"
#include "log.h"
#include "scheduler.h"
#include "midi.h"
#include "ring_buf.h"

#define programChange 0xC0
#define noteOnStatus 0x90
#define noteOffStatus 0x80
#define noteKey 69
#define noteVelocity 127

static bool enabled;
static u8 pitch;
static u8 program;
static u16 prev_joy_state;
static u16 repeat_timer;
static u16 repeat_button;

#define REPEAT_DELAY 10
#define INITIAL_DELAY 30

void comm_demo_init(void)
{
    scheduler_addFrameHandler(comm_demo_vsync);
    JOY_init();
    enabled = false;
    pitch = noteKey;
    program = 0;
    prev_joy_state = 0;
    repeat_timer = 0;
    repeat_button = 0;
    ring_buf_init();

    ring_buf_write(0xB0);
    ring_buf_write(CC_SHOW_PARAMETERS_ON_UI);
    ring_buf_write(0x7F);

    ring_buf_write(noteOnStatus);
    ring_buf_write(pitch);
    ring_buf_write(0x7F);
}

bool comm_demo_is_present(void)
{
    return true;
}

u8 comm_demo_read_ready(void)
{
    if (!enabled) {
        if (JOY_readJoypad(JOY_1) & BUTTON_A) {
            enabled = true;
        } else {
            return false;
        }
    }
    return ring_buf_can_read();
}

u8 comm_demo_read(void)
{
    u8 data;
    if (ring_buf_read(&data) == RING_BUF_OK) {
        return data;
    } else {
        return 0;
    }
}

static void send_note_off(u8 pitch)
{
    ring_buf_write(noteOffStatus);
    ring_buf_write(pitch);
    ring_buf_write(0);
}

static void send_note_on(u8 pitch)
{
    ring_buf_write(noteOnStatus);
    ring_buf_write(pitch);
    ring_buf_write(noteVelocity);
}

static void send_program_change(u8 program)
{
    ring_buf_write(programChange);
    ring_buf_write(program);
}

static bool should_repeat_button(u16 joy_state, u16 button_mask)
{
    if (joy_state & button_mask) {
        if (repeat_button != button_mask) {
            repeat_button = button_mask;
            repeat_timer = 1;
            return true;
        } else {
            repeat_timer++;
            if (repeat_timer > INITIAL_DELAY
                && (repeat_timer - INITIAL_DELAY) % REPEAT_DELAY == 0) {
                return true;
            }
        }
    } else if (repeat_button == button_mask) {
        repeat_button = 0;
        repeat_timer = 0;
    }
    return false;
}

void comm_demo_vsync(u16 delta)
{
    JOY_update();

    u16 joy_state = JOY_readJoypad(JOY_1);

    if (should_repeat_button(joy_state, BUTTON_UP)) {
        send_note_off(pitch);
        pitch++;
        if (pitch > MAX_MIDI_PITCH) {
            pitch = MAX_MIDI_PITCH;
        }
        send_note_on(pitch);
    }
    if (should_repeat_button(joy_state, BUTTON_DOWN)) {
        send_note_off(pitch);
        pitch--;
        if (pitch < MIN_MIDI_PITCH) {
            pitch = MIN_MIDI_PITCH;
        }
        send_note_on(pitch);
    }
    if (should_repeat_button(joy_state, BUTTON_RIGHT)) {
        send_note_off(pitch);
        program++;
        if (program > 0x7F) {
            program = 0x7F;
        }
        send_program_change(program);
        send_note_on(pitch);
    }
    if (should_repeat_button(joy_state, BUTTON_LEFT)) {
        send_note_off(pitch);
        program--;
        if (program == 0xFF) {
            program = 0;
        }
        send_program_change(program);
        send_note_on(pitch);
    }
    if (should_repeat_button(joy_state, BUTTON_A)) {
        send_note_off(pitch);
        send_note_on(pitch);
    }
    if (should_repeat_button(joy_state, BUTTON_B)) {
        send_note_off(pitch);
    }

    prev_joy_state = joy_state;
}

u8 comm_demo_write_ready(void)
{
    return false;
}

void comm_demo_write(u8 data)
{
    (void)data;
}
