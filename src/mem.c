#include "mem.h"

u8 mem_read_u8(size_t address)
{
    return *((vu8*)address);
}

void mem_write_u8(size_t address, u8 value)
{
    *((vu8*)address) = value;
}
