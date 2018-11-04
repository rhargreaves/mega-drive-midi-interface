#include <types.h>

u16 uintToStr(u32 value, char* str, u16 minsize);
void VDP_drawText(const char* str, u16 x, u16 y);
void YM2612_writeReg(const u16 part, const u8 reg, const u8 data);
void YM2612_enableDAC();
void SYS_disableInts();
