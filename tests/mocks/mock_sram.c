#include "mock_sram.h"
#include "cmocka_inc.h"
#include <string.h>

static u8 sram_data[0x4000];

void __wrap_SRAM_writeByte(u32 offset, u8 data)
{
    if (offset >= sizeof(sram_data)) {
        print_error("SRAM write out of bounds: %ld", offset);
        return;
    }
    sram_data[offset] = data;
}

u8* __wrap_SRAM_readByte(u32 offset)
{
    if (offset >= sizeof(sram_data)) {
        print_error("SRAM read out of bounds: %ld", offset);
        return NULL;
    }
    return &sram_data[offset];
}

u8* mock_sram_data(u32 offset)
{
    if (offset >= sizeof(sram_data)) {
        print_error("SRAM data pointer out of bounds: %ld", offset);
        return NULL;
    }
    return (u8*)&sram_data[offset];
}

void __wrap_SRAM_enable(void)
{
    function_called();
}

void __wrap_SRAM_disable(void)
{
    function_called();
}

void mock_sram_init(void)
{
    memset(sram_data, 0, sizeof(sram_data));
}

void _expect_sram_enable(const char* file, int line)
{
    expect_function_call_with_pos(__wrap_SRAM_enable, file, line);
}

void _expect_sram_disable(const char* file, int line)
{
    expect_function_call_with_pos(__wrap_SRAM_disable, file, line);
}