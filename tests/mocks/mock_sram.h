#pragma once
#include "genesis.h"

void __wrap_sram_init(void);
bool __wrap_sram_is_present(void);
void __wrap_sram_enable(bool write);
void __wrap_sram_disable(void);
void __wrap_sram_write(u32 offset, u8 data);
u8 __wrap_sram_read(u32 offset);

void mock_sram_init(void);
u8* mock_sram_data(u32 offset);
