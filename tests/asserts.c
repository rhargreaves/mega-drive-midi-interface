#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "asserts.h"
#include "wraps.h"
#include <cmocka.h>

#define REG_PART(chan) chan < 3 ? 0 : 1
#define REG_OFFSET(chan) chan % 3

void stub_comm_read_returns_midi_event(u8 status, u8 data, u8 data2)
{
    will_return(__wrap_comm_read, status);
    will_return(__wrap_comm_read, data);
    will_return(__wrap_comm_read, data2);
}

void expect_ym2612_write_reg(u8 part, u8 reg, u8 data)
{
    expect_value(__wrap_YM2612_writeReg, part, part);
    expect_value(__wrap_YM2612_writeReg, reg, reg);
    expect_value(__wrap_YM2612_writeReg, data, data);
}

void expect_ym2612_writeOperator(u8 chan, u8 op, u8 baseReg, u8 data)
{
    expect_ym2612_write_reg(
        REG_PART(chan), baseReg + REG_OFFSET(chan) + (op * 4), data);
}

void expect_ym2612_writeOperator_any_data(u8 chan, u8 op, u8 baseReg)
{
    expect_value(__wrap_YM2612_writeReg, part, REG_PART(chan));
    expect_value(
        __wrap_YM2612_writeReg, reg, baseReg + REG_OFFSET(chan) + (op * 4));
    expect_any(__wrap_YM2612_writeReg, data);
}

void expect_ym2612_write_channel(u8 chan, u8 baseReg, u8 data)
{
    expect_ym2612_write_reg(REG_PART(chan), baseReg + REG_OFFSET(chan), data);
}
