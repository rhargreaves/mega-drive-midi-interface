#pragma once
#include <types.h>

#define MAX_MIDI_CHANS 6

void midi_noteOn(u8 chan, u8 pitch, u8 velocity);
void midi_noteOff(u8 chan);
void midi_channelVolume(u8 chan, u8 volume);
void midi_pan(u8 chan, u8 pan);
