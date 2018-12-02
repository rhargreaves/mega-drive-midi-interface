#pragma once
#include <types.h>

#define MAX_PSG_CHANS 4

void psg_noteOff(u8 channel);
void psg_noteOn(u8 channel, u16 freq);
void psg_attenuation(u8 channel, u8 attenuation);
u8 psg_busy(void);
