#pragma once
#include "genesis.h"

void midi_tx_send_sysex(const u8* data, u16 length);
void midi_tx_send_cc(u8 channel, u8 controller, u8 value);
