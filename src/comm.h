#pragma once
#include <types.h>

u8 comm_read(void);
u16 comm_idleCount(void);
u16 comm_busyCount(void);
void comm_resetCounts(void);
