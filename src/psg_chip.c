#include "psg_chip.h"
#include "bits.h"
#include <psg.h>
#include <types.h>

static u8 audible;

void psg_attenuation(u8 channel, u8 attenuation)
{
    PSG_setEnvelope(channel, attenuation);
    if (attenuation == PSG_ATTENUATION_SILENCE) {
        CLEAR_BIT(audible, channel);
    } else {
        SET_BIT(audible, channel);
    }
}

u8 psg_busy(void)
{
    return audible;
}
