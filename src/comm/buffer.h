#pragma once
#include "genesis.h"

#define BUFFER_SIZE 4096

void buffer_init(void);
u8 buffer_read(void);
void buffer_write(u8 data);
bool buffer_can_read(void);
bool buffer_can_write(void);
u16 buffer_available(void);
