#pragma once
#include "types.h"

void midi_dac_note_on(u8 chan, u8 pitch, u8 velocity);
void midi_dac_pitch_bend(u8 chan, u16 bend);
void midi_dac_reset(void);
void midi_dac_note_off(u8 chan, u8 pitch);
void midi_dac_channel_volume(u8 chan, u8 volume);
void midi_dac_pan(u8 chan, u8 pan);
void midi_dac_program(u8 chan, u8 program);
void midi_dac_all_notes_off(u8 chan);
void midi_dac_pitch(u8 chan, u8 pitch, u8 cents);
