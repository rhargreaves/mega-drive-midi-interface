#include "cmocka_inc.h"
#include "comm.h"

void stub_usb_receive_byte(u8 value)
{
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, value);
}

void expect_usb_sent_byte(u8 value)
{
    will_return(__wrap_comm_everdrive_write_ready, 1);
    expect_value(__wrap_comm_everdrive_write, data, value);
}

void stub_comm_read_returns_midi_event(u8 status, u8 data, u8 data2)
{
    will_return(__wrap_comm_read, status);
    will_return(__wrap_comm_read, data);
    will_return(__wrap_comm_read, data2);
}

void expect_ym2612_write_reg_any_data(u8 part, u8 reg)
{
    expect_value(__wrap_YM2612_writeReg, part, part);
    expect_value(__wrap_YM2612_writeReg, reg, reg);
    expect_any(__wrap_YM2612_writeReg, data);
}

u8 regOpIndex(u8 op)
{
    if (op == 1) {
        return 2;
    } else if (op == 2) {
        return 1;
    } else {
        return op;
    }
}

void expect_ym2612_write_operator_any_data(u8 chan, u8 op, u8 baseReg)
{
    expect_value(__wrap_YM2612_writeReg, part, REG_PART(chan));
    expect_value(__wrap_YM2612_writeReg, reg,
        baseReg + REG_OFFSET(chan) + (regOpIndex(op) * 4));
    expect_any(__wrap_YM2612_writeReg, data);
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

void expect_ym2612_write_operator(u8 chan, u8 op, u8 baseReg, u8 data)
{
    u8 part = REG_PART(chan);
    u8 reg = baseReg + REG_OFFSET(chan) + (regOpIndex(op) * 4);

    expect_ym2612_write_reg(part, reg, data);
}

void expect_ym2612_write_reg(u8 part, u8 reg, u8 data)
{
#ifdef DEBUG
    print_message("expect: YM2612_writeReg(part=%d, reg=0x%X, data=0x%X)\n",
        part, reg, data);
#endif
    expect_value(__wrap_YM2612_writeReg, part, part);
    expect_value(__wrap_YM2612_writeReg, reg, reg);
    expect_value(__wrap_YM2612_writeReg, data, data);
}

void expect_ym2612_write_channel(u8 chan, u8 baseReg, u8 data)
{
    expect_ym2612_write_reg(
        REG_PART(chan), ((baseReg) + REG_OFFSET(chan)), data);
}
