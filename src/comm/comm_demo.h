#pragma once
#include "genesis.h"

void comm_demo_init(void);
bool comm_demo_is_present(void);
u8 comm_demo_read_ready(void);
u8 comm_demo_read(void);
u8 comm_demo_write_ready(void);
void comm_demo_write(const u8* data, u16 length);
void comm_demo_vsync(u16 delta);
