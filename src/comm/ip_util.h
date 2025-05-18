#pragma once
#include "genesis.h"

uint16_t uint8_to_str(uint8_t num, char* str);
int uint32_to_ip_str(uint32_t ip_u32, char* ip_str);

#define IP(o1, o2, o3, o4) ((o1 << 24) | (o2 << 16) | (o3 << 8) | o4)
