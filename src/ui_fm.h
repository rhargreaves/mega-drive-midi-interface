#pragma once
#include <stdint.h>
#include <types.h>
#include <stdbool.h>

void ui_fm_setMidiChannelParametersVisibility(u8 chan, bool show);
void ui_fm_init(void);
void ui_fm_update(void);
