#include "cmocka_inc.h"
#include "mocks/mock_ym2612.h"
#include "ym2612_regs.h"
#include "test_helpers.h"

static bool disableChecks = false;
static bool outputRegs = false;
static int regWrites = 0;

#define REG_PART(chan) (chan < 3 ? 0 : 1)
#define REG_OFFSET(chan) (chan % 3)
#define KEY_ON_OFF_CH_INDEX(chan) (chan < 3 ? chan : chan + 1)

void mock_ym2612_disable_checks(void)
{
    disableChecks = true;
}

void mock_ym2612_enable_checks(void)
{
    disableChecks = false;
}

void mock_ym2612_enable_reg_output(void)
{
    outputRegs = true;
    regWrites = 0;
}

void mock_ym2612_disable_reg_output(void)
{
    outputRegs = false;
}

void __wrap_YM2612_writeReg(const u16 part, const u8 reg, const u8 data)
{
    if (outputRegs) {
        print_message("call: YM2612_writeReg(part=%d, reg=0x%X, data=0x%X) #%d\n", part, reg, data,
            ++regWrites);
    }

    if (disableChecks)
        return;
    check_expected(part);
    check_expected(reg);
    check_expected(data);
}

void __wrap_YM2612_write(const u16 port, const u8 data)
{
    if (disableChecks)
        return;
    check_expected(port);
    check_expected(data);
}

void _expect_ym2612_write_reg_any_data(u8 part, u8 reg, const char* const file, const int line)
{
#ifdef DEBUG
    print_message("expect: YM2612_writeReg(part=%d, reg=0x%X, data=*)\n", part, reg);
#endif
    expect_value_with_pos(__wrap_Z80_getAndRequestBus, wait, TRUE, file, line);
    will_return_with_pos(__wrap_Z80_getAndRequestBus, false, file, line);

    expect_any_with_pos(__wrap_YM2612_writeReg, part, file, line);
    expect_any_with_pos(__wrap_YM2612_writeReg, reg, file, line);
    expect_any_with_pos(__wrap_YM2612_writeReg, data, file, line);

    expect_value_with_pos(__wrap_YM2612_write, port, 0, file, line);
    expect_value_with_pos(__wrap_YM2612_write, data, 0x2A, file, line);

    expect_function_call(__wrap_Z80_releaseBus);
}

void _expect_ym2612_write_operator_any_data(
    u8 chan, u8 op, u8 baseReg, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_Z80_getAndRequestBus, wait, TRUE, file, line);
    will_return_with_pos(__wrap_Z80_getAndRequestBus, false, file, line);

    expect_value_with_pos(__wrap_YM2612_writeReg, part, REG_PART(chan), file, line);
    expect_value_with_pos(__wrap_YM2612_writeReg, reg,
        baseReg + REG_OFFSET(chan) + (YM_OP_REG_INDEX(op) * 4), file, line);
    expect_any_with_pos(__wrap_YM2612_writeReg, data, file, line);

    expect_value_with_pos(__wrap_YM2612_write, port, 0, file, line);
    expect_value_with_pos(__wrap_YM2612_write, data, 0x2A, file, line);

    expect_function_call(__wrap_Z80_releaseBus);
}

void _expect_ym2612_write_reg_at_init(
    u8 part, u8 reg, u8 data, const char* const file, const int line)
{
#ifdef DEBUG
    print_message("expect: YM2612_writeReg(part=%d, reg=0x%X, data=0x%X)\n", part, reg, data);
#endif
    expect_value_with_pos(__wrap_Z80_getAndRequestBus, wait, TRUE, file, line);
    will_return_with_pos(__wrap_Z80_getAndRequestBus, true, file, line);

    expect_value_with_pos(__wrap_YM2612_writeReg, part, part, file, line);
    expect_value_with_pos(__wrap_YM2612_writeReg, reg, reg, file, line);
    expect_value_with_pos(__wrap_YM2612_writeReg, data, data, file, line);
}

void _expect_ym2612_write_reg_any_data_at_init(
    u8 part, u8 reg, const char* const file, const int line)
{
#ifdef DEBUG
    print_message("expect: YM2612_writeReg(part=%d, reg=0x%X, data=*)\n", part, reg);
#endif
    expect_value_with_pos(__wrap_Z80_getAndRequestBus, wait, TRUE, file, line);
    will_return_with_pos(__wrap_Z80_getAndRequestBus, true, file, line);

    expect_value_with_pos(__wrap_YM2612_writeReg, part, part, file, line);
    expect_value_with_pos(__wrap_YM2612_writeReg, reg, reg, file, line);
    expect_any_with_pos(__wrap_YM2612_writeReg, data, file, line);
}

void _expect_ym2612_write_reg(u8 part, u8 reg, u8 data, const char* const file, const int line)
{
#ifdef DEBUG
    print_message("expect: YM2612_writeReg(part=%d, reg=0x%X, data=0x%X)\n", part, reg, data);
#endif
    expect_value_with_pos(__wrap_Z80_getAndRequestBus, wait, TRUE, file, line);
    will_return_with_pos(__wrap_Z80_getAndRequestBus, false, file, line);

    expect_value_with_pos(__wrap_YM2612_writeReg, part, part, file, line);
    expect_value_with_pos(__wrap_YM2612_writeReg, reg, reg, file, line);
    expect_value_with_pos(__wrap_YM2612_writeReg, data, data, file, line);

    expect_value_with_pos(__wrap_YM2612_write, port, 0, file, line);
    expect_value_with_pos(__wrap_YM2612_write, data, 0x2A, file, line);

    expect_function_call(__wrap_Z80_releaseBus);
}

void _expect_ym2612_write_operator(
    u8 chan, u8 op, u8 baseReg, u8 data, const char* const file, const int line)
{
    u8 part = REG_PART(chan);
    u8 reg = baseReg + REG_OFFSET(chan) + (YM_OP_REG_INDEX(op) * 4);

    _expect_ym2612_write_reg(part, reg, data, file, line);
}

void _expect_ym2612_write_channel(
    u8 chan, u8 baseReg, u8 data, const char* const file, const int line)
{
    _expect_ym2612_write_reg(REG_PART(chan), ((baseReg) + REG_OFFSET(chan)), data, file, line);
}

void _expect_ym2612_write_channel_any_data(
    u8 chan, u8 baseReg, const char* const file, const int line)
{
    _expect_ym2612_write_reg_any_data(REG_PART(chan), baseReg + REG_OFFSET(chan), file, line);
}

void _expect_ym2612_write_all_operators(
    u8 chan, u8 baseReg, u8 data, const char* const file, const int line)
{
    for (u8 op = YM_OP1; op <= YM_OP4; op++) {
        _expect_ym2612_write_operator(chan, op, baseReg, data, file, line);
    }
}

void _expect_ym2612_write_all_operators_any_data(
    u8 chan, u8 baseReg, const char* const file, const int line)
{
    for (u8 op = YM_OP1; op <= YM_OP4; op++) {
        _expect_ym2612_write_operator_any_data(chan, op, baseReg, file, line);
    }
}

void _expect_ym2612_note_on(u8 chan, const char* const file, const int line)
{
    _expect_ym2612_write_reg(0, YM_KEY_ON_OFF, 0xF0 + KEY_ON_OFF_CH_INDEX(chan), file, line);
}

void _expect_ym2612_note_off(u8 chan, const char* const file, const int line)
{
    _expect_ym2612_write_reg(0, YM_KEY_ON_OFF, KEY_ON_OFF_CH_INDEX(chan), file, line);
}

void _expect_ym2612_write_frequency(u8 chan, u16 freqNumBlk, const char* const file, const int line)
{
    _expect_ym2612_write_channel(chan, YM_BASE_FREQ_MSB_BLK, (freqNumBlk >> 8), file, line);
    _expect_ym2612_write_channel(chan, YM_BASE_FREQ_LSB, (freqNumBlk & 0xFF), file, line);
}

void _expect_ym2612_write_frequency_any_data(u8 chan, const char* const file, const int line)
{
    _expect_ym2612_write_channel_any_data(chan, YM_BASE_FREQ_MSB_BLK, file, line);
    _expect_ym2612_write_channel_any_data(chan, YM_BASE_FREQ_LSB, file, line);
}

void _expect_ym2612_write_operator_volumes(
    u8 chan, const u8* volumes, u8 count, const char* const file, const int line)
{
    for (u8 op = 0; op < count; op++) {
        _expect_ym2612_write_operator(chan, op, YM_BASE_TOTAL_LEVEL, volumes[op], file, line);
    }
}
