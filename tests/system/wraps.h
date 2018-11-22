#include <midi.h>
#include <types.h>

void __wrap_YM2612_writeReg(const u16 part, const u8 reg, const u8 data);
void __wrap_VDP_drawText(const char* str, u16 x, u16 y);
void __wrap_SYS_setVIntCallback(_voidCallback* CB);
void __wrap_VDP_setTextPalette(u16 palette);
void __wrap_VDP_clearText(u16 x, u16 y, u16 w);
void __wrap_PSG_setEnvelope(u8 channel, u8 value);
void __wrap_PSG_setFrequency(u8 channel, u16 value);
u8 __wrap_ssf_usb_read(void);
u16 __wrap_ssf_usb_rd_ready(void);
