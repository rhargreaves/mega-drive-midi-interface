#pragma once
#include "types.h"

typedef struct PitchCents {
    u8 pitch;
    s8 cents;
} PitchCents;

PitchCents pc_bend(u8 pitch, s8 cents, u16 pitchBend);
PitchCents pc_bend_st(u8 pitch, s8 cents, u16 pitchBend, PitchCents range);
PitchCents pc_shift(PitchCents pc, s16 centsAdd);
