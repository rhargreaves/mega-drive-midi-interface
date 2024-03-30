#include "scheduler.h"
#include "everdrive_led.h"
#include "midi_psg.h"
#include "ui.h"
#include "midi_receiver.h"
#include "comm_megawifi.h"
#include "comm_demo.h"
#include <stdint.h>
#include <types.h>

#define MAX_TICK_HANDLERS 3
#define MAX_FRAME_HANDLERS 6

static HanderFunc* tickHandlers[MAX_TICK_HANDLERS];
static HanderFunc* frameHandlers[MAX_FRAME_HANDLERS];

static u16 previousFrame;
static volatile u16 frame;
static u16 ticks;
static u16 tickHandlersLength;
static u16 frameHandlersLength;

void scheduler_init(void)
{
    previousFrame = 0;
    frame = 0;
    ticks = 0;
    tickHandlersLength = 0;
    frameHandlersLength = 0;
    scheduler_addTickHandler(*comm_megawifi_tick);
    scheduler_addTickHandler(*midi_receiver_read_if_comm_ready);
    scheduler_addFrameHandler(*midi_psg_tick);
    scheduler_addFrameHandler(*ui_update);
    scheduler_addFrameHandler(*everdrive_led_tick);
    scheduler_addFrameHandler(*comm_megawifi_vsync);
    scheduler_addFrameHandler(*comm_demo_vsync);
}

void scheduler_vsync(void)
{
    frame++;
}

static void onFrame(void)
{
    for (u16 i = 0; i < frameHandlersLength; i++) {
        frameHandlers[i]();
    }
}

u16 scheduler_ticks(void)
{
    return ticks;
}

static void onTick(void)
{
    ticks++;
    for (u16 i = 0; i < tickHandlersLength; i++) {
        tickHandlers[i]();
    }
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

void scheduler_addTickHandler(HanderFunc* onTick)
{
    tickHandlers[tickHandlersLength++] = onTick;
}

void scheduler_addFrameHandler(HanderFunc* onFrame)
{
    frameHandlers[frameHandlersLength++] = onFrame;
}
