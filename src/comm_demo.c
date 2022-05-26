#include "comm_demo.h"
#include <joy.h>
#include <stdbool.h>
#include <sys.h>
#include "midi_fm.h"
#include "log.h"

#define noteOnStatus 0x90
#define noteOffStatus 0x80
#define noteKey 48
#define noteVelocity 127

#define NOTE_OFF_END 6
#define NOTE_ON_END 3

#define NOTE_ON_WAIT 10000
#define NOTE_OFF_WAIT 500

u8 track[] = {
    noteOnStatus,
    noteKey,
    noteVelocity,
    noteOffStatus,
    noteKey,
    noteVelocity,
};

static u8 cursor;
static u16 wait;
static bool enabled;
static u8 pitch;

void comm_demo_init(void)
{
    JOY_init();
    cursor = 0;
    wait = 0;
    enabled = false;
    pitch = noteKey;
}

u8 comm_demo_read_ready(void)
{
    if (!enabled) {
        if (JOY_readJoypad(JOY_1) & BUTTON_A) {
            enabled = true;
        }
    }
    if (!enabled) {
        return false;
    }

    if (wait == 0) {
        return true;
    } else {
        wait--;
        return false;
    }
}

u8 comm_demo_read(void)
{
    if (cursor == 0) {
        track[1] = pitch;
        track[4] = pitch;
        log_info("Demo: Pitch=%d", pitch);
    }
    u8 data = track[cursor];
    cursor++;
    if (cursor == NOTE_ON_END) {
        wait = NOTE_ON_WAIT;
    }
    if (cursor == NOTE_OFF_END) {
        wait = NOTE_OFF_WAIT;
    }
    if (cursor == NOTE_OFF_END) {
        cursor = 0;
    }
    return data;
}

void comm_demo_vsync(void)
{
    JOY_update();

    u16 curState = JOY_readJoypad(JOY_1);
    if (curState & BUTTON_UP) {
        pitch++;
        if (pitch > MAX_MIDI_PITCH) {
            pitch = MAX_MIDI_PITCH;
        }
    }
    if (curState & BUTTON_DOWN) {
        pitch--;
        if (pitch < MIN_MIDI_PITCH) {
            pitch = MIN_MIDI_PITCH;
        }
    }
}

u8 comm_demo_write_ready(void)
{
    return false;
}

void comm_demo_write(u8 data)
{
    (void)data;
}
