#pragma once
#include "vstring.h"

void debug_message(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
