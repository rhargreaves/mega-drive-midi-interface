#pragma once
#include "genesis.h"

bool settings_is_megawifi_rom(void);
bool settings_debug_load(void);
bool settings_debug_serial(void);
bool settings_debug_megawifi_init(void);
bool settings_debug_ticks(void);

#define COMM_EVERDRIVE_X7 1
#define COMM_EVERDRIVE_PRO 1
#define COMM_SERIAL 1
#define COMM_MEGAWIFI 1

#define DEFAULT_MAPPING_MODE MappingMode_Auto

#define DEBUG_MEGAWIFI_SEND 0
#define DEBUG_MEGAWIFI_INIT 0
#define DEBUG_MEGAWIFI_SYNC 0
#define DEBUG_LOAD 0
#define DEBUG_SERIAL 0
#define DEBUG_TICKS 0

// #define DEBUG_EVENTS
