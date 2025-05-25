#include "sram_ed_pro.h"
#include "mem.h"

#define SRAM_BASE_ADDR 0x200001
#define REG_SSF_CTRL 0xA130F0
#define REG_SSF_CTRL4 0xA130F8

#define SSF_CTRL_UNLOCK 0x8000
#define SSF_CTRL_MEM_WRITE 0x2000
#define BANK_SRAM 31
#define BANK4 4

void sram_ed_pro_enable(bool write)
{
    mem_write_u16(REG_SSF_CTRL, SSF_CTRL_UNLOCK | (write ? SSF_CTRL_MEM_WRITE : 0));
    mem_write_u16(REG_SSF_CTRL4, BANK_SRAM);
}

void sram_ed_pro_disable(void)
{
    mem_write_u16(REG_SSF_CTRL, SSF_CTRL_UNLOCK);
    mem_write_u16(REG_SSF_CTRL4, BANK4);
}

void sram_ed_pro_write(u32 offset, u8 value)
{
    mem_write_u8(SRAM_BASE_ADDR + offset * 2, value);
}

u8 sram_ed_pro_read(u32 offset)
{
    return mem_read_u8(SRAM_BASE_ADDR + offset * 2);
}