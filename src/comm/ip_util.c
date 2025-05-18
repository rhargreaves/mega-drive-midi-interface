#include "ip_util.h"

uint16_t uint8_to_str(uint8_t num, char* str)
{
    uint16_t i = 0;
    uint16_t tmp;

    // Compute digits and write decimal number
    // On 3 digit numbers, first one can only be 1 or 2. Take advantage of
    // this to avoid division (test if this is really faster).
    if (num > 199) {
        str[i++] = '2';
        num -= 200;
    } else if (num > 99) {
        str[i++] = '1';
        num -= 100;
    }

    tmp = num / 10;
    if (tmp) {
        str[i++] = '0' + tmp;
    }
    str[i++] = '0' + num % 10;
    str[i] = '\0';

    return i;
}

/// Converts an IP address in uint32_t binary representation to
int uint32_to_ip_str(uint32_t ip_u32, char* ip_str)
{
    uint8_t* byte = (uint8_t*)&ip_u32;
    int pos = 0;
    int i;

    for (i = 0; i < 3; i++) {
        pos += uint8_to_str(byte[i], ip_str + pos);
        ip_str[pos++] = '.';
    }
    pos += uint8_to_str(byte[i], ip_str + pos);
    ip_str[pos] = '\0';

    return pos;
}
