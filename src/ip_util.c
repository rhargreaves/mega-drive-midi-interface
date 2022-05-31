#include "ip_util.h"

static const char* str_to_uint8(const char* str, uint8_t* result)
{
    uint8_t i;

    *result = 0;

    // Skip leading zeros
    while (*str == '0')
        str++;
    // Special case: number is zero
    if (*str < '0' || *str > '9')
        return str;
    // Determine number length (up to 4 characters)
    for (i = 0; (i < 4) && (str[i] >= '0') && (str[i] <= '9'); i++)
        ;

    switch (i) {
    // If number is 3 characters, the number fits in 8 bits only if
    // lower than 256
    case 3:
        if ((str[0] > '2')
            || ((str[0] == '2')
                && ((str[1] > '5') || ((str[1] == '5') && (str[2] > '5')))))
            return NULL;
        else {
            *result = ((*str) - '0') * 100;
            str++;
        }
        // fallthrough
    case 2:
        *result += ((*str) - '0') * 10;
        str++;
        // fallthrough
    case 1:
        *result += (*str) - '0';
        str++;
        break;

    // If length is 4 or more, number does not fit in 8 bits.
    default:
        return NULL;
    }
    return str;
}

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

/// Convert an IPv4 address in string format to binary format
uint32_t ip_str_to_uint32(const char* ip)
{
    uint32_t bin_ip;
    uint8_t* byte = (uint8_t*)&bin_ip;
    int i;

    for (i = 0; i < 4; i++) {
        if ((ip = str_to_uint8(ip, &byte[i])) == NULL) {
            return 0;
        }
        ip++;
    }
    return bin_ip;
}
