#pragma once
#include "genesis.h"
#include "synth.h"

#define MAX_FM_CHAN 5
#define MIN_MIDI_PITCH 11
#define MAX_MIDI_PITCH 106

typedef struct PercussionPreset {
    FmPreset preset;
    u8 key;
} PercussionPreset;

#define PACK_BIG_ENDIAN __attribute__((packed, scalar_storage_order("big-endian")))

typedef struct PACK_BIG_ENDIAN SramOperator {
    u8 multiple : 4;
    u8 detune : 3;
    u8 attackRate : 5;
    u8 rateScaling : 2;
    u8 decayRate : 5;
    u8 amplitudeModulation : 1;
    u8 sustainLevel : 4;
    u8 sustainRate : 5;
    u8 releaseRate : 4;
    u8 totalLevel : 7;
    u8 ssgEg : 4;
} SramOperator;

typedef struct PACK_BIG_ENDIAN SramFmPreset {
    u8 algorithm : 3;
    u8 feedback : 3;
    u8 ams : 2;
    u8 fms : 3;
    SramOperator operators[MAX_FM_OPERATORS];
} SramFmPreset;

typedef struct PACK_BIG_ENDIAN SramFmPresetSlot {
    u16 magic_number;
    u8 version;
    SramFmPreset preset;
    u8 reserved[5];
    u16 checksum;
} SramFmPresetSlot;

void midi_fm_init(const FmPreset** defPresets, const PercussionPreset** defaultPercussionPresets);
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
void midi_fm_store_preset(u8 program, const FmPreset* preset);
void midi_fm_clear_preset(u8 program);