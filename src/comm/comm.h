#pragma once
#include "genesis.h"

typedef enum CommMode { Discovery, Everdrive, EverdrivePro, Serial, MegaWiFi, Demo } CommMode;
typedef enum CommStatus { COMM_OK, COMM_NOT_READY, COMM_TIMEOUT } CommStatus;

#define COMM_DEFAULT_ATTEMPTS 1000

void comm_init(void);
void comm_write(u8 data);
bool comm_read_ready(void);
CommStatus comm_read(u8* data, u16 attempts);
u16 comm_idle_count(void);
u16 comm_busy_count(void);
void comm_reset_counts(void);
CommMode comm_mode(void);
