#include "scheduler.h"
#include "ui.h"
#include <types.h>

static u16 previousFrame;
static volatile u16 frame;

void scheduler_init(void)
{
    previousFrame = 0;
    frame = 0;
}

void scheduler_vsync(void)
{
    frame++;
}

static void onFrame(void)
{
    ui_update();
}

void scheduler_doEvents(void)
{
    if (frame != previousFrame) {
        onFrame();
        previousFrame = frame;
    }
}
