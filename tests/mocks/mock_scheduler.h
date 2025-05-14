#pragma once
#include "genesis.h"
#include "cmocka.h"
#include "scheduler.h"

void __wrap_scheduler_tick(void);
void __wrap_scheduler_addTickHandler(TickHandlerFunc* onTick);
void __wrap_scheduler_addFrameHandler(FrameHandlerFunc* onFrame);
void __wrap_scheduler_yield(void);
extern void __real_scheduler_init(void);
extern void __real_scheduler_tick(void);
extern void __real_scheduler_addTickHandler(TickHandlerFunc* onTick);
extern void __real_scheduler_addFrameHandler(FrameHandlerFunc* onFrame);
extern void __real_scheduler_yield(void);

void expect_scheduler_yield(void);