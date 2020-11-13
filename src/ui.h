#pragma once
#include <stdint.h>
#include <types.h>
#include <stdbool.h>

#ifndef BUILD
#define BUILD "*Dev Build*"
#endif

#define MAX_Y 27
#define MAX_X 39
#define MARGIN_X 1
#define MARGIN_Y 1

void ui_init(void);
void ui_update(void);
void ui_drawText(const char* text, u16 x, u16 y);
void ui_showLogs(void);
void ui_hideLogs(void);
