#pragma once
#include "genesis.h"

#define MIN_PSG_CHAN 6
#define MAX_PSG_CHAN 9

#define EEF_END 0xFF
#define EEF_LOOP_START 0xFE
#define EEF_LOOP_END 0xFD

#define MAX_PSG_CHANS 4

#define PSG_ATTENUATION_SILENCE 0xF
#define PSG_ATTENUATION_LOUDEST 0
#define MAX_ATTENUATION PSG_ATTENUATION_SILENCE

void midi_psg_init(const u8** defaultEnvelopes);
void midi_psg_reset(void);
void midi_psg_note_on(u8 chan, u8 pitch, s8 cents, u8 velocity);
void midi_psg_note_off(u8 chan, u8 pitch);
void midi_psg_all_notes_off(u8 chan);
void midi_psg_channel_volume(u8 chan, u8 volume);
void midi_psg_program(u8 chan, u8 program);
void midi_psg_pan(u8 chan, u8 pan);
void midi_psg_tick(u16 delta);
void midi_psg_load_envelope(const u8* eef);
void midi_psg_pitch(u8 chan, u8 pitch, s8 cents);
