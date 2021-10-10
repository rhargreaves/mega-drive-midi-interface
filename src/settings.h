#pragma once
#include <stdbool.h>

bool settings_isMegaWiFiRom(void);
bool settings_debugLoad(void);

#define COMM_EVERDRIVE_X7 1
#define COMM_EVERDRIVE_PRO 1
#define COMM_SERIAL 1
#define COMM_MEGAWIFI 1

#define DEBUG_MEGAWIFI_SEND 0
#define DEBUG_MEGAWIFI_INIT 0
#define DEBUG_MEGAWIFI_SYNC 0
#define DEBUG_LOAD 0

// #define DEBUG_TICKS
// #define DEBUG_EVENTS
