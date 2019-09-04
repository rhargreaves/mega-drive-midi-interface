#include "midi_psg.h"
#include "psg_chip.h"
#include <stdbool.h>

#define MIN_PSG_CHAN 6
#define MAX_PSG_CHAN 9

static u8 psgChannel(u8 midiChannel);

static const u16 FREQUENCIES[] = { 8, 9, 9, 10, 10, 11, 12, 12, 13, 14, 15, 15,
    16, 17, 18, 19, 21, 22, 23, 25, 26, 28, 29, 31, 33, 35, 37, 39, 41, 44, 46,
    49, 52, 55, 58, 62, 65, 69, 73, 78, 82, 87, 93, 98, 104, 110, 117, 123, 131,
    139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247, 262, 277, 294, 311,
    330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740,
    784, 831, 880, 932, 988, 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568,
    1661, 1760, 1865, 1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136,
    3322, 3520, 3729, 3951, 4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272,
    6645, 7040, 7459, 7902, 8372, 8870, 9397, 9956, 10548, 11175, 11840,
    12544 };

static const u8 ATTENUATIONS[] = { 15, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12,
    11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7,
    7, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static u8 pitches[MAX_PSG_CHANS];
static u8 attenuations[] = { PSG_ATTENUATION_LOUDEST, PSG_ATTENUATION_LOUDEST,
    PSG_ATTENUATION_LOUDEST, PSG_ATTENUATION_LOUDEST };
static bool notesOn[MAX_PSG_CHAN];

static u8 psgChannel(u8 midiChannel)
{
    return midiChannel - MIN_PSG_CHAN;
}

void midi_psg_noteOn(u8 chan, u8 pitch, u8 velocity)
{
    u8 psgChan = psgChannel(chan);
    pitches[psgChan] = pitch;
    psg_frequency(psgChan, FREQUENCIES[pitch]);
    psg_attenuation(psgChan, attenuations[psgChan]);
    notesOn[psgChan] = true;
}

void midi_psg_noteOff(u8 chan, u8 pitch)
{
    u8 psgChan = psgChannel(chan);
    psg_attenuation(psgChan, PSG_ATTENUATION_SILENCE);
    notesOn[psgChan] = false;
}

void midi_psg_channelVolume(u8 chan, u8 volume)
{
    u8 psgChan = psgChannel(chan);
    attenuations[psgChan] = ATTENUATIONS[volume];
    if (notesOn[psgChan]) {
        psg_attenuation(psgChan, attenuations[psgChan]);
    }
}

void midi_psg_pitchBend(u8 chan, u16 bend)
{
    u8 psgChan = psgChannel(chan);
    u8 origPitch = pitches[psgChan];
    u16 freq = FREQUENCIES[origPitch];

    s16 bendRelative = bend - 0x2000;
    freq = freq + (bendRelative / 100);

    psg_frequency(psgChan, freq);
}

void midi_psg_program(u8 chan, u8 program)
{
}

void midi_psg_reset(void)
{
    for(u8 chan = 0; chan < MAX_PSG_CHANS; chan++)
    {
        notesOn[chan] = false;
        attenuations[chan] = PSG_ATTENUATION_LOUDEST;
    }
}
