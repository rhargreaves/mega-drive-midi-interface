#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "asserts.h"
#include "wraps.h"
#include <cmocka.h>

#define REG_PART(chan) chan < 3 ? 0 : 1
#define REG_OFFSET(chan) chan % 3

void stub_usb_receive_byte(u8 value)
{
    will_return(__wrap_ssf_usb_rd_ready, 1);
    will_return(__wrap_ssf_usb_read, value);
}

void expect_usb_sent_byte(u8 value)
{
    will_return(__wrap_ssf_usb_wr_ready, 1);
    expect_value(__wrap_ssf_usb_write, data, value);
    will_return(__wrap_ssf_usb_write, value);
}

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

void expect_ym2612_write_reg_any_data(u8 part, u8 reg)
{
    expect_value(__wrap_YM2612_writeReg, part, part);
    expect_value(__wrap_YM2612_writeReg, reg, reg);
    expect_any(__wrap_YM2612_writeReg, data);
}

void expect_ym2612_write_operator(u8 chan, u8 op, u8 baseReg, u8 data)
{
    expect_ym2612_write_reg(
        REG_PART(chan), baseReg + REG_OFFSET(chan) + (op * 4), data);
}

void expect_ym2612_write_operator_any_data(u8 chan, u8 op, u8 baseReg)
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

void expect_ym2612_write_channel_any_data(u8 chan, u8 baseReg)
{
    expect_ym2612_write_reg_any_data(
        REG_PART(chan), baseReg + REG_OFFSET(chan));
}

void expect_synth_pitch_any(void)
{
    expect_any(__wrap_synth_pitch, channel);
    expect_any(__wrap_synth_pitch, octave);
    expect_any(__wrap_synth_pitch, freqNumber);
}

void expect_synth_pitch(u8 channel, u8 octave, u16 freqNumber)
{
    expect_value(__wrap_synth_pitch, channel, channel);
    expect_value(__wrap_synth_pitch, octave, octave);
    expect_value(__wrap_synth_pitch, freqNumber, freqNumber);
}
