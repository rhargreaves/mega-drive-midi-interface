#include <fm.h>
#include <synth.h>

void synth_init(void)
{
    fm_writeReg(0, 0x27, 0); // Ch 3 Normal
    fm_writeReg(0, 0x28, 0); // All channels off
    fm_writeReg(0, 0x28, 1);
    fm_writeReg(0, 0x28, 2);
    fm_writeReg(0, 0x28, 4);
    fm_writeReg(0, 0x28, 5);
    fm_writeReg(0, 0x28, 6);
    fm_writeReg(0, 0x30, 0x71); // DT1/MUL
    fm_writeReg(0, 0x34, 0x0D);
    fm_writeReg(0, 0x38, 0x33);
    fm_writeReg(0, 0x3C, 0x01);
    fm_writeReg(0, 0x40, 0x23); // Total Level
    fm_writeReg(0, 0x44, 0x2D);
    fm_writeReg(0, 0x48, 0x26);
    fm_writeReg(0, 0x4C, 0x00);
    fm_writeReg(0, 0x50, 0x5F); // RS/AR
    fm_writeReg(0, 0x54, 0x99);
    fm_writeReg(0, 0x58, 0x5F);
    fm_writeReg(0, 0x5C, 0x99);
    fm_writeReg(0, 0x60, 5); // AM/D1R
    fm_writeReg(0, 0x64, 5);
    fm_writeReg(0, 0x68, 5);
    fm_writeReg(0, 0x6C, 7);
    fm_writeReg(0, 0x70, 2); // D2R
    fm_writeReg(0, 0x74, 2);
    fm_writeReg(0, 0x78, 2);
    fm_writeReg(0, 0x7C, 2);
    fm_writeReg(0, 0x80, 0x11); // D1L/RR
    fm_writeReg(0, 0x84, 0x11);
    fm_writeReg(0, 0x88, 0x11);
    fm_writeReg(0, 0x8C, 0xA6);
    fm_writeReg(0, 0x90, 0); // Proprietary
    fm_writeReg(0, 0x94, 0);
    fm_writeReg(0, 0x98, 0);
    fm_writeReg(0, 0x9C, 0);
    fm_writeReg(0, 0xB0, 0x32); // feedback/algor
    fm_writeReg(0, 0xB4, 0xC0);
    fm_writeReg(0, 0xA4, 0x22); // freq
    fm_writeReg(0, 0xA0, 0x69);
}

void synth_noteOn(u8 channel)
{
    fm_writeReg(0, 0x28, 0xF0);
}

void synth_noteOff(u8 channel)
{
}
