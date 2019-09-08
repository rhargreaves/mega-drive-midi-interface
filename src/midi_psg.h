#pragma once
#include <types.h>

#define MIN_PSG_CHAN 6
#define MAX_PSG_CHAN 9

void midi_psg_init(void);
void midi_psg_noteOn(u8 chan, u8 pitch, u8 velocity);
void midi_psg_noteOff(u8 chan, u8 pitch);
void midi_psg_channelVolume(u8 chan, u8 volume);
void midi_psg_pitchBend(u8 chan, u16 bend);
void midi_psg_program(u8 chan, u8 program);
void midi_psg_reset(void);


