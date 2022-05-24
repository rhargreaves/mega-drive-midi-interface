#include "comm_demo.h"
#include <joy.h>
#include <stdbool.h>
#include <sys.h>

#define noteOnStatus 0x90
#define noteOffStatus 0x80
#define noteKey 48
#define noteVelocity 127

const u8 track[] = {
    noteOnStatus,
    noteKey,
    noteVelocity,
    noteOffStatus,
    noteKey,
    noteVelocity,
};

u8 cursor;
u16 wait;
bool enabled;

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
    if (cursor == 3) {
        wait = 19000;
    }
    if (cursor == 6) {
        wait = 1000;
    }
    if (cursor == 6) {
        cursor = 0;
    }
    return data;
}

u8 comm_demo_write_ready(void)
{
    return FALSE;
}

void comm_demo_write(u8 data)
{
    (void)data;
}
