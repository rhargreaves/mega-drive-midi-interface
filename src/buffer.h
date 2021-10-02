#pragma once
#include <types.h>
#include <stdbool.h>

#define BUFFER_SIZE 2048

void buffer_init(void);
u8 buffer_read(void);
void buffer_write(u8 data);
u8 buffer_can_read(void);
bool buffer_can_write(void);
u16 buffer_available(void);
