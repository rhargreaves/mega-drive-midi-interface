#pragma once
#include <types.h>

void comm_serial_init(void);
u8 comm_serial_read_ready(void);
u8 comm_serial_read(void);
u8 comm_serial_write_ready(void);
void comm_serial_write(u8 data);
