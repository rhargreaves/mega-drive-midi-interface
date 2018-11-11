#include "psg_chip.h"
#include <psg.h>
#include <types.h>

#define ATTENUATION_OFF 0xF
#define ATTENUATION_ON 0

static void applyAttenuation(u8 channel);

static u8 attenuations[] = {
    ATTENUATION_ON,
    ATTENUATION_ON,
    ATTENUATION_ON,
    ATTENUATION_ON
};

static void applyAttenuation(u8 channel)
{
    PSG_setEnvelope(channel, attenuations[channel]);
}

void psg_noteOff(u8 channel)
{
    PSG_setEnvelope(channel, ATTENUATION_OFF);
}

void psg_noteOn(u8 channel, u16 freq)
{
    PSG_setFrequency(channel, freq);
    applyAttenuation(channel);
}

void psg_attenuation(u8 channel, u8 attenuation)
{
    attenuations[channel] = attenuation;
    applyAttenuation(channel);
}
