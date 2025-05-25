#include "genesis.h"

void sram_ed_pro_enable(bool write);
void sram_ed_pro_disable(void);
void sram_ed_pro_write(u32 offset, u8 value);
u8 sram_ed_pro_read(u32 offset);