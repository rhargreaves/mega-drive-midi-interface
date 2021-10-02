#pragma once
#include "synth.h"
#include <stdbool.h>
#include <types.h>

#define MAX_FM_CHAN 5

typedef struct PercussionPreset PercussionPreset;

struct PercussionPreset {
    FmChannel channel;
    u8 key;
};

void midi_fm_init(const FmChannel** defaultPresets,
    const PercussionPreset** defaultPercussionPresets);
void midi_fm_note_on(u8 chan, u8 pitch, u8 velocity);
void midi_fm_note_off(u8 chan, u8 pitch);
void midi_fm_channel_volume(u8 chan, u8 volume);
void midi_fm_pan(u8 chan, u8 pan);
void midi_fm_pitch_bend(u8 chan, u16 bend);
void midi_fm_program(u8 chan, u8 program);
void midi_fm_all_notes_off(u8 chan);
void midi_fm_percussive(u8 chan, bool enabled);
