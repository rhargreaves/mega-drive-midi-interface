#pragma once
#include <types.h>

void midi_psg_noteOn(u8 chan, u8 pitch, u8 velocity);
void midi_psg_noteOff(u8 chan);
void midi_psg_channelVolume(u8 chan, u8 volume);
