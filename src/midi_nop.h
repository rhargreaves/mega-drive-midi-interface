#pragma once
#include <types.h>

void midi_nop_noteOn(u8 chan, u8 pitch, u8 velocity);
void midi_nop_noteOff(u8 chan, u8 pitch);
void midi_nop_channelVolume(u8 chan, u8 volume);
void midi_nop_pitchBend(u8 chan, u16 bend);
void midi_nop_program(u8 chan, u8 program);
