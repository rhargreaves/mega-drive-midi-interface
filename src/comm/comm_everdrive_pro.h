#pragma once
#include "genesis.h"

void comm_everdrive_pro_init(void);
bool comm_everdrive_pro_is_present(void);
u8 comm_everdrive_pro_read_ready(void);
u8 comm_everdrive_pro_read(void);
u8 comm_everdrive_pro_write_ready(void);
void comm_everdrive_pro_write(u8 data);
