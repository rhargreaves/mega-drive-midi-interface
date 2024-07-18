#pragma once
#include "types.h"

typedef void(HandlerFunc)(void);

void scheduler_init(void);
void scheduler_vsync(void);
void scheduler_tick(void);
void scheduler_run(void);
u16 scheduler_ticks(void);
void scheduler_addTickHandler(HandlerFunc* onTick);
void scheduler_addFrameHandler(HandlerFunc* onFrame);
