#pragma once
#include "genesis.h"

u8 __wrap_mem_read_u8(size_t address);
void __wrap_mem_write_u8(size_t address, u8 value);
u16 __wrap_mem_read_u16(size_t address);
void __wrap_mem_write_u16(size_t address, u16 value);

extern u8 __real_mem_read_u8(size_t address);
extern void __real_mem_write_u8(size_t address, u8 value);
extern u16 __real_mem_read_u16(size_t address);
extern void __real_mem_write_u16(size_t address, u16 value);

void expect_mem_read_u8(size_t address, u8 expected_value);
void expect_mem_write_u8(size_t address, u8 value);
void expect_mem_read_u16(size_t address, u16 expected_value);
void expect_mem_write_u16(size_t address, u16 value);
