#include "comm_demo.h"
#include <joy.h>
#include <stdbool.h>
#include <sys.h>

#define noteOnStatus 0x90
#define noteOffStatus 0x80
#define noteKey 48
#define noteVelocity 127

#define NOTE_OFF_END 6
#define NOTE_ON_END 3

#define NOTE_ON_WAIT 10000
#define NOTE_OFF_WAIT 500

const u8 track[] = {
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

void comm_demo_init(void)
{
    JOY_init();
    cursor = 0;
    wait = 0;
    enabled = false;
}

u8 comm_demo_read_ready(void)
{
    if (!enabled) {
        JOY_update();
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

u8 comm_demo_write_ready(void)
{
    return false;
}

void comm_demo_write(u8 data)
{
    (void)data;
}
