#pragma once
#define SET_BIT(var, pos) var |= 1 << pos;
#define CLEAR_BIT(var, pos) var &= ~(1 << pos);
#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))
