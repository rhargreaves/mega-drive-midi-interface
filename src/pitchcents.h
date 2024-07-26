#pragma once
#include "types.h"

typedef struct PitchCents {
    u8 pitch;
    s8 cents;
} PitchCents;

PitchCents pitchcents_bend(u8 pitch, s8 cents, u16 pitchBend);
PitchCents pitchcents_shift(PitchCents pc, s16 centsAdd);
