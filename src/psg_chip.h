#pragma once
#include <types.h>

#define MAX_PSG_CHANS 4

#define PSG_ATTENUATION_SILENCE 0xF
#define PSG_ATTENUATION_LOUDEST 0
#define MAX_ATTENUATION PSG_ATTENUATION_SILENCE

void psg_noteOff(u8 channel);
void psg_noteOn(u8 channel, u16 freq);
void psg_attenuation(u8 channel, u8 attenuation);
void psg_frequency(u8 channel, u16 freq);
u8 psg_busy(void);
