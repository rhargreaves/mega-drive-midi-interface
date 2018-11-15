#pragma once
#include <types.h>

typedef struct ControlChange ControlChange;

struct ControlChange {
    u8 controller;
    u8 value;
};

void interface_loop(void);
void interface_tick(void);
void interface_init(void);
u8 interface_lastUnknownStatus(void);
ControlChange* interface_lastUnknownCC(void);
