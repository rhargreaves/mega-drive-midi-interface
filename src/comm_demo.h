#pragma once
#include <stdint.h>
#include <types.h>

void comm_demo_init(void);
u8 comm_demo_read_ready(void);
u8 comm_demo_read(void);
u8 comm_demo_write_ready(void);
void comm_demo_write(u8 data);
void comm_demo_vsync(void);
