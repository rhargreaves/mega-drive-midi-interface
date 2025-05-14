#pragma once
#include "genesis.h"

typedef void(FrameHandlerFunc)(u16 delta);
typedef void(TickHandlerFunc)(void);

void scheduler_init(void);
void scheduler_vsync(void);
void scheduler_tick(void);
void scheduler_run(void);
u16 scheduler_ticks(void);
void scheduler_addTickHandler(TickHandlerFunc* onTick);
void scheduler_addFrameHandler(FrameHandlerFunc* onFrame);
void scheduler_yield(void);