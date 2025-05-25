#include "genesis.h"

void sram_init(void);
void sram_enable(bool write);
void sram_disable(void);
void sram_write(u32 offset, u8 value);
u8 sram_read(u32 offset);
bool sram_is_present(void);