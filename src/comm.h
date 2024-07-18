#pragma once
#include "types.h"

enum CommMode { Discovery, Everdrive, EverdrivePro, Serial, MegaWiFi, Demo };
typedef enum CommMode CommMode;

void comm_init(void);
void comm_write(u8 data);
bool comm_read_ready(void);
u8 comm_read(void);
u16 comm_idle_count(void);
u16 comm_busy_count(void);
void comm_reset_counts(void);
CommMode comm_mode(void);
