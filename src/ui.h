#pragma once
#include "types.h"

#define MAX_Y 27
#define MAX_X 39
#define MARGIN_X 1
#define MARGIN_Y 1

void ui_init(void);
void ui_update(void);
void ui_draw_text(const char* text, u16 x, u16 y);
void ui_show_logs(void);
void ui_hide_logs(void);
