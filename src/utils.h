#pragma once
#include "genesis.h"

#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))
#define LENGTH_OF(array) (sizeof(array) / sizeof(array[0]))

#define PACK_BIG_ENDIAN __attribute__((packed, scalar_storage_order("big-endian")))

s16 memcmp(const void* data1, const void* data2, size_t count);
void debug_message(const char* fmt, ...) __attribute__((format(printf, 1, 2)));