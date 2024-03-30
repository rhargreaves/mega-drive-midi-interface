#pragma once
#include <stdint.h>
#include <types.h>

void comm_megawifi_init(void);
u8 comm_megawifi_read_ready(void);
u8 comm_megawifi_read(void);
u8 comm_megawifi_write_ready(void);
void comm_megawifi_write(u8 data);

void comm_megawifi_tick(void);
void comm_megawifi_midiEmitCallback(u8 data);
void comm_megawifi_send(u8 ch, char* data, u16 len);
void comm_megawifi_vsync(void);

typedef enum MegaWifiStatus MegaWifiStatus;

enum MegaWifiStatus { NotDetected, Detected, Listening, Connected };

MegaWifiStatus comm_megawifi_status(void);
