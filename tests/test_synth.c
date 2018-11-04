#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <types.h>

static void synth_init_sets_initial_registers(void** state)
{
    u16 count = 39;
    expect_any_count(__wrap_fm_writeReg, part, count);
    expect_any_count(__wrap_fm_writeReg, reg, count);
    expect_any_count(__wrap_fm_writeReg, data, count);

    __real_synth_init();
}

static void synth_writes_fm_reg(void** state)
{
    expect_value(__wrap_fm_writeReg, part, 0);
    expect_value(__wrap_fm_writeReg, reg, 0x28);
    expect_value(__wrap_fm_writeReg, data, 0xF0);

    __real_synth_noteOn(0);
}
