#include "sram.h"
#include "sram_ed_pro.h"
#include "sram_ed_x7.h"

#define ED_PRO 0
#define ED_X7 1

void sram_enable(bool write)
{
#if ED_PRO
    sram_ed_pro_enable(write);
#elif ED_X7
    sram_ed_x7_enable(write);
#endif
}

void sram_disable(void)
{
#if ED_PRO
    sram_ed_pro_disable();
#elif ED_X7
    sram_ed_x7_disable();
#endif
}

void sram_write(u32 offset, u8 value)
{
#if ED_PRO
    sram_ed_pro_write(offset, value);
#elif ED_X7
    sram_ed_x7_write(offset, value);
#endif
}

u8 sram_read(u32 offset)
{
#if ED_PRO
    return sram_ed_pro_read(offset);
#elif ED_X7
    return sram_ed_x7_read(offset);
#endif
}