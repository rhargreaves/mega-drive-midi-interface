#pragma once
#include "types.h"

void comm_everdrive_init(void);
u8 comm_everdrive_read_ready(void);
u8 comm_everdrive_read(void);
u8 comm_everdrive_write_ready(void);
void comm_everdrive_write(u8 data);
