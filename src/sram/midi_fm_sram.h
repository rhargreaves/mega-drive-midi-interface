#pragma once
#include "genesis.h"
#include "midi_fm.h"
#include "utils.h"

#define SRAM_FM_PRESET_LENGTH 36

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

_Static_assert(
    sizeof(SramFmPresetSlot) == SRAM_FM_PRESET_LENGTH, "Unexpected SramFmPresetSlot size");

void midi_fm_sram_init(void);
void midi_fm_sram_save_preset(u8 program, const FmPreset* preset);
void midi_fm_sram_clear_preset(u8 program);
void midi_fm_sram_load_presets(FmPreset* const userPresets, FmPreset** const activeUserPresets);