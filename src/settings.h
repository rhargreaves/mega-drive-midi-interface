#pragma once
#include "genesis.h"

bool settings_is_megawifi_rom(void);
bool settings_debug_load(void);
bool settings_debug_serial(void);
bool settings_debug_megawifi_init(void);
bool settings_debug_ticks(void);
const u8* settings_startup_midi_sequence(size_t* length);

#define COMM_EVERDRIVE_X7 1
#define COMM_EVERDRIVE_PRO 1
#define COMM_SERIAL 1
#define COMM_MEGAWIFI 1

#define DEBUG_MEGAWIFI_SEND 0
#define DEBUG_MEGAWIFI_INIT 0
#define DEBUG_MEGAWIFI_SYNC 0
#define DEBUG_LOAD 0
#define DEBUG_SERIAL 0
#define DEBUG_TICKS 0
// #define DEBUG_EVENTS
