#pragma once
#include "genesis.h"

#define PORT2_CTRL 0xA1000B
#define PORT2_SCTRL 0xA10019
#define PORT2_TX 0xA10015
#define PORT2_RX 0xA10017

#define EXT_CTRL 0xA1000D
#define EXT_SCTRL 0xA1001F
#define EXT_TX 0xA1001B
#define EXT_RX 0xA1001D

#define SCTRL_SIN 0x20
#define SCTRL_SOUT 0x10
#define SCTRL_300_BPS 0xC0
#define SCTRL_1200_BPS 0x80
#define SCTRL_2400_BPS 0x40
#define SCTRL_4800_BPS 0x00

#define SCTRL_TFUL 0x1
#define SCTRL_RRDY 0x2
#define SCTRL_RERR 0x4
#define SCTRL_RINT 0x8

#define CTRL_PCS_OUT 0x7F

typedef enum IoPort {
    IoPort_Ctrl2,
    IoPort_Ext,
} IoPort;

void serial_init(IoPort port, u8 sctrlFlags);
void serial_setReadyToReceiveCallback(VoidCallback* cb);
bool serial_readyToReceive(void);
u8 serial_receive(void);
u8 serial_sctrl(void);
void serial_send(u8 data);
bool serial_readyToSend(void);
void serial_sendWhenReady(u8 data);
