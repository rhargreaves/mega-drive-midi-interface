#pragma once
#include <stdbool.h>
#include <types.h>

void interface_loop(void);
void interface_tick(void);
void interface_init(void);
u8 interface_lastUnknownStatus(void);
bool interface_polyphonic(void);
u16 interface_beat(void);
