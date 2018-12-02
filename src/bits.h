#pragma once
#define SET_BIT(var, bit) var |= 1 << bit;
#define CLEAR_BIT(var, bit) var &= ~(1 << bit);
