#include "psg_chip.h"
#include "bits.h"
#include <psg.h>
#include <types.h>

#define ATTENUATION_SILENCE 0xF
#define ATTENUATION_LOUDEST 0

static void applyAttenuation(u8 channel);

static u8 attenuations[] = { ATTENUATION_LOUDEST, ATTENUATION_LOUDEST,
    ATTENUATION_LOUDEST, ATTENUATION_LOUDEST };

static u8 noteOn;

static void applyAttenuation(u8 channel)
{
    PSG_setEnvelope(channel, attenuations[channel]);
}

void psg_noteOff(u8 channel)
{
    PSG_setEnvelope(channel, ATTENUATION_SILENCE);
    CLEAR_BIT(noteOn, channel);
}

void psg_noteOn(u8 channel, u16 freq)
{
    PSG_setFrequency(channel, freq);
    applyAttenuation(channel);
    SET_BIT(noteOn, channel);
}

void psg_attenuation(u8 channel, u8 attenuation)
{
    attenuations[channel] = attenuation;
    applyAttenuation(channel);
}

void psg_frequency(u8 channel, u16 freq)
{
    PSG_setFrequency(channel, freq);
}

u8 psg_busy(void)
{
    return noteOn;
}
