#pragma once
#include "synth.h"
#include <types.h>

#define MAX_FM_CHAN 5

void midi_fm_init(Channel** defaultPresets);
void midi_fm_noteOn(u8 chan, u8 pitch, u8 velocity);
void midi_fm_noteOff(u8 chan, u8 pitch);
void midi_fm_channelVolume(u8 chan, u8 volume);
void midi_fm_pan(u8 chan, u8 pan);
void midi_fm_pitchBend(u8 chan, u16 bend);
void midi_fm_program(u8 chan, u8 program);
void midi_fm_allNotesOff(u8 chan);
