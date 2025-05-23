#pragma once
#include "genesis.h"
#include "synth.h"

#define MAX_FM_CHAN 5
#define MIN_MIDI_PITCH 11
#define MAX_MIDI_PITCH 106

typedef struct PercussionPreset {
    FmChannel channel;
    u8 key;
} PercussionPreset;

void midi_fm_init(const FmChannel** defPresets, const PercussionPreset** defaultPercussionPresets);
void midi_fm_reset(void);
void midi_fm_note_on(u8 chan, u8 pitch, s8 cents, u8 velocity);
void midi_fm_note_off(u8 chan, u8 pitch);
void midi_fm_channel_volume(u8 chan, u8 volume);
void midi_fm_pan(u8 chan, u8 pan);
void midi_fm_program(u8 chan, u8 program);
void midi_fm_all_notes_off(u8 chan);
void midi_fm_percussive(u8 chan, bool enabled);
u8 midi_fm_pitch_to_octave(u8 pitch);
void midi_fm_pitch(u8 chan, u8 pitch, s8 cents);
u16 midi_fm_pitch_cents_to_freq_num(u8 pitch, s8 cents);
void midi_fm_store_preset(u8 program, const FmChannel* preset);