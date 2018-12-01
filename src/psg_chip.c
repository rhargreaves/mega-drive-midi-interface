#include "psg_chip.h"
#include <psg.h>
#include <types.h>

#define ATTENUATION_SILENCE 0xF
#define ATTENUATION_LOUDEST 0

static void setAttenuation(u8 channel);

static u8 attenuations[] = { ATTENUATION_LOUDEST, ATTENUATION_LOUDEST,
    ATTENUATION_LOUDEST, ATTENUATION_LOUDEST };

static void setAttenuation(u8 channel)
{
    PSG_setEnvelope(channel, attenuations[channel]);
}

void psg_noteOff(u8 channel)
{
    PSG_setEnvelope(channel, ATTENUATION_SILENCE);
}

void psg_noteOn(u8 channel, u16 freq)
{
    PSG_setFrequency(channel, freq);
    setAttenuation(channel);
}

void psg_attenuation(u8 channel, u8 attenuation)
{
    attenuations[channel] = attenuation;
    setAttenuation(channel);
}
