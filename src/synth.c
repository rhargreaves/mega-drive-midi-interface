#include "synth.h"
#include <ym2612.h>

static void synth_writeFm(u8 channel, u8 baseReg, u8 data);
static u8 synth_keyOnOffRegOffset(u8 channel);

void synth_init(void)
{
    YM2612_writeReg(0, 0x27, 0); // Ch 3 Normal
    for (u8 chan = 0; chan < MAX_SYNTH_CHANS; chan++) {
        synth_noteOff(chan);
        synth_writeFm(chan, 0x30, 0x71); // DT1/MUL
        synth_writeFm(chan, 0x34, 0x0D);
        synth_writeFm(chan, 0x38, 0x33);
        synth_writeFm(chan, 0x3C, 0x01);
        synth_writeFm(chan, 0x40, 0x23); // Total Level
        synth_writeFm(chan, 0x44, 0x2D);
        synth_writeFm(chan, 0x48, 0x26);
        synth_writeFm(chan, 0x4C, 0x00);
        synth_writeFm(chan, 0x50, 0x5F); // RS/AR
        synth_writeFm(chan, 0x54, 0x99);
        synth_writeFm(chan, 0x58, 0x5F);
        synth_writeFm(chan, 0x5C, 0x99);
        synth_writeFm(chan, 0x60, 5); // AM/D1R
        synth_writeFm(chan, 0x64, 5);
        synth_writeFm(chan, 0x68, 5);
        synth_writeFm(chan, 0x6C, 7);
        synth_writeFm(chan, 0x70, 2); // D2R
        synth_writeFm(chan, 0x74, 2);
        synth_writeFm(chan, 0x78, 2);
        synth_writeFm(chan, 0x7C, 2);
        synth_writeFm(chan, 0x80, 0x11); // D1L/RR
        synth_writeFm(chan, 0x84, 0x11);
        synth_writeFm(chan, 0x88, 0x11);
        synth_writeFm(chan, 0x8C, 0xA6);
        synth_writeFm(chan, 0xB0, 0x32); // feedback/algor
        synth_writeFm(chan, 0xB4, 0xC0);
        synth_writeFm(chan, 0xA4, 0x22); // freq
        synth_writeFm(chan, 0xA0, 0x69);
    }
    YM2612_writeReg(0, 0x90, 0); // Proprietary
    YM2612_writeReg(0, 0x94, 0);
    YM2612_writeReg(0, 0x98, 0);
    YM2612_writeReg(0, 0x9C, 0);
}

static void synth_writeFm(u8 channel, u8 baseReg, u8 data)
{
    YM2612_writeReg(channel > 2 ? 1 : 0, baseReg + (channel % 3), data);
}

static u8 synth_keyOnOffRegOffset(u8 channel)
{
    return (channel < 3) ? channel : (channel + 1);
}

void synth_noteOn(u8 channel)
{
    YM2612_writeReg(0, 0x28, 0xF0 + synth_keyOnOffRegOffset(channel));
}

void synth_noteOff(u8 channel)
{
    YM2612_writeReg(0, 0x28, synth_keyOnOffRegOffset(channel));
}

void synth_pitch(u8 channel, u8 octave, u16 freqNumber)
{
    synth_writeFm(channel, 0xA4, (freqNumber >> 8) | (octave << 3));
    synth_writeFm(channel, 0xA0, freqNumber);
}

void synth_totalLevel(u8 channel, u8 totalLevel)
{
    synth_writeFm(channel, 0x4C, totalLevel);
}

void synth_stereo(u8 channel, u8 mode)
{
    synth_writeFm(channel, 0xB4, mode << 6);
}

void synth_algorithm(u8 channel, u8 algorithm)
{
    synth_writeFm(channel, 0xB0, algorithm);
}

void synth_operatorTotalLevel(u8 channel, u8 op, u8 totalLevel)
{
    synth_writeFm(channel, 0x40 + (op * 4), totalLevel);
}
