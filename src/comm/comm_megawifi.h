#pragma once
#include "genesis.h"

void comm_megawifi_init(void);
bool comm_megawifi_is_present(void);
u8 comm_megawifi_read_ready(void);
u8 comm_megawifi_read(void);
u8 comm_megawifi_write_ready(void);
void comm_megawifi_write(u8 data);
void comm_megawifi_tick(void);
void comm_megawifi_midiEmitCallback(u8 data);
void comm_megawifi_send(u8 ch, char* data, u16 len);
void comm_megawifi_vsync(u16 delta);

typedef enum MegaWifiStatus MegaWifiStatus;

enum MegaWifiStatus { NotDetected, Initialising, Listening, Connected };

MegaWifiStatus comm_megawifi_status(void);
