
#include "sys_wraps.h"
#include "types.h"
#include "vdp.h"
#include <stdbool.h>

#ifndef UNIT_TESTS
extern void __real_SYS_disableInts(void);
extern void __real_SYS_enableInts(void);

#define VDP_MODE_REG_1 0x00
#define VDP_MODE_REG_2 0x01

#define VDP_IE0_ENABLE_VINT 0x20
#define VDP_IE1_ENABLE_HINT 0x10

void __wrap_SYS_disableInts(void)
{
    VDP_setReg(
        VDP_MODE_REG_1, VDP_getReg(VDP_MODE_REG_1) & ~VDP_IE1_ENABLE_HINT);
    VDP_setReg(
        VDP_MODE_REG_2, VDP_getReg(VDP_MODE_REG_2) & ~VDP_IE0_ENABLE_VINT);
}

void __wrap_SYS_enableInts(void)
{
    VDP_setReg(
        VDP_MODE_REG_1, VDP_getReg(VDP_MODE_REG_1) | VDP_IE1_ENABLE_HINT);
    VDP_setReg(
        VDP_MODE_REG_2, VDP_getReg(VDP_MODE_REG_2) | VDP_IE0_ENABLE_VINT);
}

void __wrap_JOY_update()
{
}

#endif
