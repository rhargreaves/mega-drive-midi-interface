#include "crc.h"

/*
CRC-CCITT (0xFFFF)
*/
u16 crc_calc_crc16(const u8* data_p, size_t length)
{
    u8 x;
    u16 crc = 0xFFFF;

    while (length--) {
        x = crc >> 8 ^ *data_p++;
        x ^= x >> 4;
        crc = (crc << 8) ^ ((u16)(x << 12)) ^ ((u16)(x << 5)) ^ ((u16)x);
    }
    return crc;
}