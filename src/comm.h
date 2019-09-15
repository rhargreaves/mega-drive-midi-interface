#pragma once
#include <types.h>

void comm_write(u8 data);
u8 comm_read(void);
u16 comm_idleCount(void);
u16 comm_busyCount(void);
void comm_resetCounts(void);
