
#include "util.h"

#include <memory.h>
#include <vstring.h>

const char* str_is_uint8(const char* str)
{
    uint8_t i;

    // Skip leading zeros
    while (str[0] == '0' && (str[1] >= '0' && str[1] <= '9')) { str++; }
    // Determine number length (up to 4 characters)
    for (i = 0; (i < 4) && (str[i] >= '0') && (str[i] <= '9'); i++)
        ;

    switch (i) {
    // If number is 3 characters, the number fits in 8 bits only if
    // lower than 255
    case 3:
        if ((str[0] > '2')
            || ((str[0] == '2')
                && ((str[1] > '5') || ((str[1] == '5') && (str[2] > '5'))))) {
            return NULL;
        }

        // If length is 2 or 1 characters, the number fits in 8 bits.
        // fallthrough
    case 2:
    case 1:
        return str + i;

        // If length is 4 or more, number does not fit in 8 bits.
    default:
        return NULL;
    }
}

int ip_validate(const char* str)
{
    int8_t i;

    // Evaluate if we have 4 numbers fitting in a byte, separated by '.'
    if (!(str = str_is_uint8(str)))
        return FALSE;

    for (i = 2; i >= 0; i--) {
        if (*str != '.') {
            return FALSE;
        }
        str++;
        if (!(str = str_is_uint8(str))) {
            return FALSE;
        }
    }

    if (*str != '\0') {
        return FALSE;
    }
    return TRUE;
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

uint8_t uint16_to_str(uint16_t num, char* str)
{
    int8_t i, j;
    char digit[5] = { 0 };

    // Maximum is 65535
    for (i = 4; i > 0 && num; i--) {
        digit[i] = num % 10;
        num /= 10;
    }
    digit[0] = num;

    // Skip leading zeros
    for (i = 0; i < 4 && !digit[i]; i++)
        ;

    // Convert digits to characters
    for (j = 0; i < 5; i++, j++) { str[j] = digit[i] + '0'; }
    str[j] = '\0';

    return j;
}

uint8_t int16_to_str(int16_t num, char* str)
{
    uint8_t off = 0;

    if (num < 0) {
        *str++ = '-';
        off++;
    }

    return uint16_to_str(num, str) + off;
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

int8_t int8_to_str(int8_t num, char* str)
{
    int i = 0;

    if (num < 0) {
        num = -num;
        str[i++] = '-';
    }
    i += uint8_to_str(num, str + i);

    return i;
}

const char* str_to_uint8(const char* str, uint8_t* result)
{
    uint8_t i;

    *result = 0;

    // Skip leading zeros
    while (*str == '0') str++;
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

void uint8_to_hex_str(uint8_t num, char* str)
{
    const char map[] = "0123456789ABCDEF";

    str[0] = map[num >> 4];
    str[1] = map[num & 0x0F];
    str[2] = '\0';
}

int uint32_to_hex_str(uint32_t num, char* str, int pad)
{
    const char map[] = "0123456789ABCDEF";
    int i;
    int nibble;
    int off;

    for (i = 0, nibble = 7; nibble >= 0; nibble--) {
        off = nibble << 2;
        if ((num >> off) & 0xF) {
            str[i++] = map[(num >> off) & 0xF];
        } else if ((i > 0) || (nibble < pad) || ((!i) && (!nibble))) {
            str[i++] = '0';
        }
    }
    str[i] = '\0';
    return i;
}

int version_to_str(const uint8_t version[3], char* str)
{
    int pos = 0;

    for (int i = 0; i < 3; i++) {
        pos += uint8_to_str(version[i], str + pos);
        str[pos++] = '.';
    }
    str[--pos] = '\0';

    return pos;
}

int version_cmp(const uint8_t old[3], const uint8_t new[3])
{
    int result;
    int i;

    for (i = 0; i < 2; i++) {
        result = new[i] - old[i];
        if (result) {
            return result;
        }
    }

    return new[i] - old[i];
}

uint16_t concat_strings(
    const char** str, uint8_t num_strs, char* output, uint16_t max_len)
{
    uint16_t pos = 0;
    int str_len;

    for (uint8_t i = 0; i < num_strs; i++) {
        if (!str[i]) {
            return 0;
        }
        str_len = v_strlen(str[i]) + 1;
        if ((pos + str_len) > max_len) {
            return 0;
        }
        memcpy(output + pos, str[i], str_len);
        pos += str_len;
    }

    return pos;
}

uint16_t concat_kv_pairs(const char** key, const char** value,
    uint8_t num_pairs, char* output, uint16_t max_len)
{
    uint16_t pos = 0;
    int key_len, value_len;

    for (uint8_t i = 0; i < num_pairs; i++) {
        if (!key[i] || !value[i]) {
            return 0;
        }
        key_len = v_strlen(key[i]) + 1;
        value_len = v_strlen(value[i]) + 1;
        if ((pos + key_len + value_len) > max_len) {
            return 0;
        }
        memcpy(output + pos, key[i], key_len);
        pos += key_len;
        memcpy(output + pos, value[i], value_len);
        pos += value_len;
    }

    return pos;
}
