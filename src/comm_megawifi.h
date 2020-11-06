#pragma once
#include <stdint.h>
#include <types.h>

void comm_megawifi_init(void);
void midi_emit(u8 data);
u8 comm_megawifi_readReady(void);
u8 comm_megawifi_read(void);
u8 comm_megawifi_writeReady(void);
void comm_megawifi_write(u8 data);
