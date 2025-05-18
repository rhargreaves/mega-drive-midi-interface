#include "mock_mem.h"
#include "cmocka_inc.h"

u8 __wrap_mem_read_u8(size_t address)
{
    check_expected(address);
    return mock_type(u8);
}

void __wrap_mem_write_u8(size_t address, u8 value)
{
    check_expected(address);
    check_expected(value);
}

u16 __wrap_mem_read_u16(size_t address)
{
    check_expected(address);
    return mock_type(u16);
}

void __wrap_mem_write_u16(size_t address, u16 value)
{
    check_expected(address);
    check_expected(value);
}

void expect_mem_write_u8(size_t address, u8 value)
{
    expect_value(__wrap_mem_write_u8, address, address);
    expect_value(__wrap_mem_write_u8, value, value);
}

void expect_mem_read_u8(size_t address, u8 expected_value)
{
    expect_value(__wrap_mem_read_u8, address, address);
    will_return(__wrap_mem_read_u8, expected_value);
}

void expect_mem_read_u16(size_t address, u16 expected_value)
{
    expect_value(__wrap_mem_read_u16, address, address);
    will_return(__wrap_mem_read_u16, expected_value);
}

void expect_mem_write_u16(size_t address, u16 value)
{
    expect_value(__wrap_mem_write_u16, address, address);
    expect_value(__wrap_mem_write_u16, value, value);
}
