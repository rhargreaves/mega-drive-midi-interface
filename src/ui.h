#pragma once
#include "genesis.h"

#define MAX_Y 27
#define MAX_X 39
#define MARGIN_X 1
#define MARGIN_Y 1
#define MAX_EFFECTIVE_X (MAX_X - MARGIN_X - MARGIN_X)
#define MAX_EFFECTIVE_Y (MAX_Y - MARGIN_Y - MARGIN_Y)

void ui_init(void);
void ui_draw_text(const char* text, u16 x, u16 y);