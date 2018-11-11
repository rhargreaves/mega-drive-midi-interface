#include "psg_chip.h"
#include <psg.h>
#include <types.h>

void psg_noteOff(u8 channel)
{
    PSG_setEnvelope(channel, 0xF);
}

void psg_noteOn(u8 channel, u16 freq, u8 attenuation)
{
    PSG_setFrequency(channel, freq);
    PSG_setEnvelope(channel, attenuation);
}
