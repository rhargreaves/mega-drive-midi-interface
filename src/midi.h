#pragma once
#include "midi_fm.h"
#include "midi_psg.h"
#include <types.h>

#define MAX_MIDI_CHANS 9

void midi_noteOn(u8 chan, u8 pitch, u8 velocity);
void midi_noteOff(u8 chan, u8 pitch);
void midi_channelVolume(u8 chan, u8 volume);
void midi_pan(u8 chan, u8 pan);
void midi_pitchBend(u8 chan, u16 bend);
