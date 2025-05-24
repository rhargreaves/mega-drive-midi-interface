#pragma once
#include "genesis.h"

void __wrap_SRAM_writeByte(u32 offset, u8 data);
u8* __wrap_SRAM_readByte(u32 offset);
void __wrap_SRAM_enable(void);
void __wrap_SRAM_disable(void);

void mock_sram_init(void);
u8* mock_sram_data(u32 offset);
