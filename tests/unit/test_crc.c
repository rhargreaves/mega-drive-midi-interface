#include "test_crc.h"
#include "crc.h"

// Validated against https://www.lammertbies.nl/comm/info/crc-calculation

void test_crc_calculate_crc16(UNUSED void** state)
{
    const u8 data[] = { 0x01, 0x02, 0x03, 0x04 };

    u16 crc = crc_calc_crc16(data, sizeof(data));

    assert_int_equal(crc, 0x89C3);
}

void test_crc_calculate_another_crc16(UNUSED void** state)
{
    const u8 data[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB,
        0xCC, 0xDD, 0xEE, 0xFF };

    u16 crc = crc_calc_crc16(data, sizeof(data));

    assert_int_equal(crc, 0x7842);
}