#include "mock_sram.h"
#include "cmocka_inc.h"
#include <string.h>

static u8 sram_data[0x1000];
static u16 write_count;
static u16 read_count;

void __wrap_SRAM_writeByte(u32 offset, u8 data)
{
    if (offset >= sizeof(sram_data)) {
        print_error("SRAM write out of bounds: %ld", offset);
        return;
    }
    sram_data[offset] = data;
    write_count++;
}

void __wrap_SRAM_readByte(u32 offset, u8* data)
{
    if (offset >= sizeof(sram_data)) {
        print_error("SRAM read out of bounds: %ld", offset);
        return;
    }
    *data = sram_data[offset];
    read_count++;
}

u8* mock_sram_data(u32 offset)
{
    if (offset >= sizeof(sram_data)) {
        print_error("SRAM data pointer out of bounds: %ld", offset);
        return NULL;
    }
    return &sram_data[offset];
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
    write_count = 0;
    read_count = 0;
}

u16 mock_sram_write_count()
{
    return write_count;
}

u16 mock_sram_read_count()
{
    return read_count;
}

void mock_sram_dump(u32 offset, u32 length)
{
    for (u32 i = 0; i < length; i++) {
        print_error("SRAM[%ld] = %02X\n", offset + i, sram_data[offset + i]);
    }
}

void _expect_sram_enable(const char* file, int line)
{
    expect_function_call_with_pos(__wrap_SRAM_enable, file, line);
}

void _expect_sram_disable(const char* file, int line)
{
    expect_function_call_with_pos(__wrap_SRAM_disable, file, line);
}