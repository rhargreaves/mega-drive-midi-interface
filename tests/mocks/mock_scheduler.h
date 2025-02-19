#pragma once
#include "genesis.h"
#include "cmocka.h"
#include "scheduler.h"

void __wrap_scheduler_tick(void);
void __wrap_scheduler_addTickHandler(HandlerFunc* onTick);
void __wrap_scheduler_addFrameHandler(HandlerFunc* onFrame);
extern void __real_scheduler_init(void);
extern void __real_scheduler_tick(void);
extern void __real_scheduler_addTickHandler(HandlerFunc* onTick);
extern void __real_scheduler_addFrameHandler(HandlerFunc* onFrame);
