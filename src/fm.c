#include <genesis.h>

void fm_writeReg(u16 part, u8 reg, u8 data)
{
    YM2612_writeReg(part, reg, data);
}
