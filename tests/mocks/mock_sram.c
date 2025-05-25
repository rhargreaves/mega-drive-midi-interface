#include "mock_sram.h"
#include "cmocka_inc.h"
#include <string.h>

static u8 sram_data[0x4000];
static bool enabled;

void __wrap_SRAM_writeByte(u32 offset, u8 data)
{
    if (!enabled) {
        print_error("SRAM write attempted when disabled");
        return;
    }

    if (offset >= sizeof(sram_data)) {
        print_error("SRAM write out of bounds: %ld", offset);
        return;
    }
    sram_data[offset] = data;
}

u8 __wrap_SRAM_readByte(u32 offset)
{
    if (!enabled) {
        print_error("SRAM read attempted when disabled");
        return 0;
    }

    if (offset >= sizeof(sram_data)) {
        print_error("SRAM read out of bounds: %ld", offset);
        return 0;
    }
    return sram_data[offset];
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
    enabled = true;
}

void __wrap_SRAM_disable(void)
{
    enabled = false;
}

void mock_sram_init(void)
{
    enabled = false;
    memset(sram_data, 0, sizeof(sram_data));
}
