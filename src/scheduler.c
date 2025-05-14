#include "scheduler.h"
#include "comm/everdrive_led.h"
#include "midi_psg.h"
#include "ui.h"
#include "midi_rx.h"
#include "comm/comm_megawifi.h"
#include "comm/comm_demo.h"

#define MAX_TICK_HANDLERS 3
#define MAX_FRAME_HANDLERS 6

static TickHandlerFunc* tickHandlers[MAX_TICK_HANDLERS];
static FrameHandlerFunc* frameHandlers[MAX_FRAME_HANDLERS];

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
}

void scheduler_vsync(void)
{
    frame++;
}

static void on_frame(u16 delta)
{
    for (u16 i = 0; i < frameHandlersLength; i++) {
        frameHandlers[i](delta);
    }
}

u16 scheduler_ticks(void)
{
    return ticks;
}

static void on_tick(void)
{
    ticks++;
    for (u16 i = 0; i < tickHandlersLength; i++) {
        tickHandlers[i]();
    }
}

void scheduler_yield(void)
{
    scheduler_tick();
}

void scheduler_tick(void)
{
    on_tick();
    if (frame != previousFrame) {
        on_frame(frame - previousFrame);
        previousFrame = frame;
    }
}

void scheduler_run(void)
{
    while (TRUE) {
        scheduler_tick();
    }
}

void scheduler_addTickHandler(TickHandlerFunc* onTick)
{
    if (tickHandlersLength == MAX_TICK_HANDLERS) {
        SYS_die("Too many tick handlers registered.");
        return;
    }
    tickHandlers[tickHandlersLength++] = onTick;
}

void scheduler_addFrameHandler(FrameHandlerFunc* onFrame)
{
    if (frameHandlersLength == MAX_FRAME_HANDLERS) {
        SYS_die("Too many frame handlers registered.");
        return;
    }
    frameHandlers[frameHandlersLength++] = onFrame;
}
