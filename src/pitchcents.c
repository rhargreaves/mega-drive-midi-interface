#include "pitchcents.h"
#include "midi.h"

PitchCents pitchcents_bend(u8 pitch, s8 cents, u16 pitchBend)
{
    s16 centsAdd = ((pitchBend - MIDI_PITCH_BEND_CENTRE) * 25) / 1024;
    PitchCents pc = { .pitch = pitch, .cents = cents };
    return pitchcents_shift(pc, centsAdd);
}

PitchCents pitchcents_shift(PitchCents pc, s16 centsAdd)
{
    u16 totalCents = (pc.pitch * 100) + pc.cents;
    totalCents += centsAdd;

    pc.pitch = totalCents / 100;
    pc.cents = totalCents % 100;
    return pc;
}
