#include "pitchcents.h"
#include "midi.h"

PitchCents pc_bend(u8 pitch, s8 cents, u16 pitchBend, PitchCents range)
{
    s16 centsRange = range.pitch * 100 + range.cents;
    s16 centsAdd = ((pitchBend - MIDI_PITCH_BEND_CENTRE) * centsRange) / 8192;
    PitchCents pc = { .pitch = pitch, .cents = cents };
    return pc_shift(pc, centsAdd);
}

PitchCents pc_shift(PitchCents pc, s16 centsAdd)
{
    u16 totalCents = (pc.pitch * 100) + pc.cents;
    totalCents += centsAdd;

    pc.pitch = totalCents / 100;
    pc.cents = totalCents % 100;
    return pc;
}
