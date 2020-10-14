#pragma once
#include <stdint.h>
#include <types.h>

#include <stdbool.h>

typedef enum CommMode CommMode;

enum CommMode { Discovery, Everdrive, EverdrivePro, Serial };

void comm_init(void);
void comm_write(u8 data);
bool comm_readReady(void);
u8 comm_read(void);
u16 comm_idleCount(void);
u16 comm_busyCount(void);
void comm_resetCounts(void);
CommMode comm_mode(void);
