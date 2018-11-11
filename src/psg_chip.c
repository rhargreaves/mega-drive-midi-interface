#include "psg_chip.h"
#include <psg.h>
#include <types.h>

static u8 attenuations[] = { 0, 0, 0, 0 };

void psg_noteOff(u8 channel)
{
    PSG_setEnvelope(channel, 0xF);
}

void psg_noteOn(u8 channel, u16 freq)
{
    PSG_setFrequency(channel, freq);
    PSG_setEnvelope(channel, attenuations[channel]);
}

void psg_attenuation(u8 channel, u8 attenuation)
{
    attenuations[channel] = attenuation;
    PSG_setEnvelope(channel, attenuations[channel]);
}
