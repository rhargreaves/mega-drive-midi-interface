#include "genesis.h"

void sram_ed_x7_enable(bool write);
void sram_ed_x7_disable(void);
void sram_ed_x7_write(u32 offset, u8 value);
u8 sram_ed_x7_read(u32 offset);