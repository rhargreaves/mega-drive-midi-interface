#pragma once
#include <types.h>

#define MAX_FM_CHAN 5

void midi_fm_noteOn(u8 chan, u8 pitch, u8 velocity);
void midi_fm_noteOff(u8 chan);
void midi_fm_channelVolume(u8 chan, u8 volume);
void midi_fm_pan(u8 chan, u8 pan);
