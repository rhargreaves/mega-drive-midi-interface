#include "scheduler.h"
#include "everdrive_led.h"
#include "midi_psg.h"
#include "ui.h"
#include "midi_receiver.h"
#include "comm_megawifi.h"
#include <stdint.h>
#include <types.h>

static u16 previousFrame;
static volatile u16 frame;
static u16 ticks;

void scheduler_init(void)
{
    previousFrame = 0;
    frame = 0;
    ticks = 0;
}

void scheduler_vsync(void)
{
    frame++;
}

static void onFrame(void)
{
    midi_psg_tick();
    ui_update();
    everdrive_led_tick();
    comm_megawifi_vsync();
}

u16 scheduler_ticks(void)
{
    return ticks;
}

static void onTick(void)
{
    ticks++;
    comm_megawifi_tick();
    midi_receiver_read_if_comm_ready();
}

void scheduler_tick(void)
{
    onTick();
    if (frame != previousFrame) {
        onFrame();
        previousFrame = frame;
    }
}

void scheduler_run(void)
{
    while (TRUE) {
        scheduler_tick();
    }
}
