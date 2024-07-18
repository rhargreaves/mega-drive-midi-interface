#pragma once
#include "types.h"

void comm_everdrive_pro_init(void);
u8 comm_everdrive_pro_read_ready(void);
u8 comm_everdrive_pro_read(void);
u8 comm_everdrive_pro_write_ready(void);
void comm_everdrive_pro_write(u8 data);
