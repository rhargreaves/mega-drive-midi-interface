#pragma once
#include "genesis.h"

void __wrap_SRAM_writeByte(u32 offset, u8 data);
void __wrap_SRAM_readByte(u32 offset, u8* data);
void __wrap_SRAM_enable(void);
void __wrap_SRAM_disable(void);

void mock_sram_init(void);
u16 mock_sram_write_count(void);
u16 mock_sram_read_count(void);
u8* mock_sram_data(u32 offset);

void _expect_sram_enable(const char* file, int line);
void _expect_sram_disable(const char* file, int line);

#define expect_sram_enable() _expect_sram_enable(__FILE__, __LINE__)
#define expect_sram_disable() _expect_sram_disable(__FILE__, __LINE__)