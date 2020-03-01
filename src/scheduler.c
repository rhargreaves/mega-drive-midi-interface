#include "scheduler.h"
#include "ui.h"
#include <types.h>

static u16 previousFrame = 0;
static volatile u16 frame = 0;

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
