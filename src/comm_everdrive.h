#pragma once
#include <stdint.h>
#include <types.h>

void comm_everdrive_init(void);
u8 comm_everdrive_readReady(void);
u8 comm_everdrive_read(void);
u8 comm_everdrive_writeReady(void);
void comm_everdrive_write(u8 data);
