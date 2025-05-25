#include "mock_sram.h"
#include "cmocka_inc.h"
#include <string.h>

static u8 sram_data[0x4000];
static bool enabled;
static bool is_init = false;
static bool is_present;
static bool write_enabled;

void mock_sram_init(void)
{
    is_present = true;
    enabled = false;
    write_enabled = false;
    is_init = true;
    memset(sram_data, 0, sizeof(sram_data));
}

void __wrap_sram_init(void)
{
    mock_sram_init();
}

static bool check_init(void)
{
    if (!is_init) {
        fail_msg("Mock SRAM not initialized");
        return false;
    }
    return true;
}

bool __wrap_sram_is_present(void)
{
    return check_init() && is_present;
}

void __wrap_sram_enable(bool write)
{
    if (!check_init()) {
        return;
    }

    enabled = true;
    write_enabled = write;
}

void __wrap_sram_disable(void)
{
    if (!check_init()) {
        return;
    }

    enabled = false;
}

u8 __wrap_sram_read(u32 offset)
{
    if (!check_init()) {
        return 0;
    }

    if (!enabled) {
        fail_msg("SRAM read attempted when disabled");
        return 0;
    }

    if (offset >= sizeof(sram_data)) {
        fail_msg("SRAM read out of bounds: %ld", offset);
        return 0;
    }
    return sram_data[offset];
}

void __wrap_sram_write(u32 offset, u8 data)
{
    if (!check_init()) {
        return;
    }

    if (!enabled) {
        fail_msg("SRAM write attempted when disabled");
        return;
    }

    if (!write_enabled) {
        fail_msg("SRAM write attempted when write is disabled");
        return;
    }

    if (offset >= sizeof(sram_data)) {
        fail_msg("SRAM write out of bounds: %ld", offset);
        return;
    }
    sram_data[offset] = data;
}

u8* mock_sram_data(u32 offset)
{
    if (!check_init()) {
        return NULL;
    }

    if (offset >= sizeof(sram_data)) {
        fail_msg("SRAM data pointer out of bounds: %ld", offset);
        return NULL;
    }
    return (u8*)&sram_data[offset];
}