#pragma once
#include "genesis.h"

void midi_fm_sm_note_on(u8 chan, u8 pitch, s8 cents, u8 velocity);
void midi_fm_sm_reset(void);
void midi_fm_sm_pitch(u8 chan, u8 pitch, s8 cents);

// no-ops
void midi_fm_sm_note_off(u8 chan, u8 pitch);
void midi_fm_sm_channel_volume(u8 chan, u8 volume);
void midi_fm_sm_pan(u8 chan, u8 pan);
void midi_fm_sm_program(u8 chan, u8 program);
void midi_fm_sm_all_notes_off(u8 chan);
