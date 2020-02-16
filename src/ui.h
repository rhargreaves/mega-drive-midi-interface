#pragma once
#include <types.h>

#include <stdbool.h>

#ifndef BUILD
#define BUILD "*Dev Build*"
#endif

void ui_init(void);
void ui_vsync(void);
void ui_update(void);
void ui_setMidiChannelParametersVisibility(u8 chan, bool show);
