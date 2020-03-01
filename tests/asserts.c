#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "asserts.h"
#include "comm.h"
#include "wraps.h"

#define REG_PART(chan) chan < 3 ? 0 : 1
#define REG_OFFSET(chan) chan % 3

void stub_usb_receive_byte(u8 value)
{
    will_return(__wrap_comm_everdrive_readReady, 1);
    will_return(__wrap_comm_everdrive_read, value);
}

void expect_usb_sent_byte(u8 value)
{
    will_return(__wrap_comm_everdrive_writeReady, 1);
    expect_value(__wrap_comm_everdrive_write, data, value);
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

void expect_synth_volume(u8 channel, u8 volume)
{
    expect_value(__wrap_synth_volume, channel, channel);
    expect_value(__wrap_synth_volume, volume, volume);
}

void expect_synth_volume_any(void)
{
    expect_any(__wrap_synth_volume, channel);
    expect_any(__wrap_synth_volume, volume);
}

void expect_psg_attenuation(u8 chan, u8 attenu)
{
    expect_value(__wrap_psg_attenuation, channel, chan);
    expect_value(__wrap_psg_attenuation, attenuation, attenu);
}
