#include "cmocka_inc.h"
#include "asserts.h"
#include "comm/comm.h"

void stub_megawifi_as_not_present(void)
{
    will_return(__wrap_mw_uart_is_present, false);
}

void stub_everdrive_as_present(void)
{
    will_return(__wrap_comm_everdrive_is_present, true);
}

void stub_usb_receive_nothing(void)
{
    will_return(__wrap_comm_everdrive_read_ready, 0);
}

void stub_usb_receive_byte(u8 value)
{
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, value);
}

void stub_usb_receive_program(u8 chan, u8 program)
{
    stub_usb_receive_byte(0xC0 + chan);
    stub_usb_receive_byte(program);
}

void stub_usb_receive_cc(u8 chan, u8 cc, u8 value)
{
    stub_usb_receive_byte(0xB0 + chan);
    stub_usb_receive_byte(cc);
    stub_usb_receive_byte(value);
}

void stub_usb_receive_note_on(u8 chan, u8 key, u8 velocity)
{
    stub_usb_receive_byte(0x90 + chan);
    stub_usb_receive_byte(key);
    stub_usb_receive_byte(velocity);
}

void stub_usb_receive_note_off(u8 chan, u8 key)
{
    stub_usb_receive_byte(0x80 + chan);
    stub_usb_receive_byte(key);
    stub_usb_receive_byte(0);
}

void stub_usb_receive_pitch_bend(u8 chan, u16 bend)
{
    u8 lower = bend & 0x007F;
    u8 upper = bend >> 7;

    stub_usb_receive_byte(0xE0 + chan);
    stub_usb_receive_byte(lower);
    stub_usb_receive_byte(upper);
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

void expect_ym2612_write_operator_any_data(u8 chan, u8 op, u8 baseReg)
{
    expect_value(__wrap_Z80_getAndRequestBus, wait, TRUE);
    will_return(__wrap_Z80_getAndRequestBus, false);

    expect_value(__wrap_YM2612_writeReg, part, REG_PART(chan));
    expect_value(__wrap_YM2612_writeReg, reg, baseReg + REG_OFFSET(chan) + (regOpIndex(op) * 4));
    expect_any(__wrap_YM2612_writeReg, data);

    expect_value(__wrap_YM2612_write, port, 0);
    expect_value(__wrap_YM2612_write, data, 0x2A);

    expect_function_call(__wrap_Z80_releaseBus);
}

void _expect_synth_pitch_any(const char* const file, const int line)
{
    debug_message("expect: synth_pitch(channel=*, octave=*, freqNumber=*)\n");
    expect_any_with_pos(__wrap_synth_pitch, channel, file, line);
    expect_any_with_pos(__wrap_synth_pitch, octave, file, line);
    expect_any_with_pos(__wrap_synth_pitch, freqNumber, file, line);
}

void _expect_synth_pitch(
    u8 channel, u8 octave, u16 freqNumber, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_pitch, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_pitch, octave, octave, file, line);
    expect_value_with_pos(__wrap_synth_pitch, freqNumber, freqNumber, file, line);
}

void expect_synth_volume(u8 channel, u8 volume)
{
    expect_value(__wrap_synth_volume, channel, channel);
    expect_value(__wrap_synth_volume, volume, volume);
}

void expect_synth_noteOn(u8 chan)
{
    expect_value(__wrap_synth_noteOn, channel, chan);
}

void expect_synth_noteOff(u8 chan)
{
    expect_value(__wrap_synth_noteOff, channel, chan);
}

void _expect_ym2612_write_reg(u8 part, u8 reg, u8 data, const char* const file, const int line)
{
#ifdef DEBUG
    print_message("expect: YM2612_writeReg(part=%d, reg=0x%X, data=0x%X)\n", part, reg, data);
#endif
    expect_value(__wrap_Z80_getAndRequestBus, wait, TRUE);
    will_return(__wrap_Z80_getAndRequestBus, false);

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
    u8 reg = baseReg + REG_OFFSET(chan) + (regOpIndex(op) * 4);

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

static bool operator_equality_check(Operator* l, Operator* r)
{
    return ((l->amplitudeModulation == r->amplitudeModulation) && (l->attackRate == r->attackRate)
        && (l->decayRate == r->decayRate) && (l->detune == r->detune)
        && (l->multiple == r->multiple) && (l->rateScaling == r->rateScaling)
        && (l->releaseRate == r->releaseRate) && (l->ssgEg == r->ssgEg)
        && (l->sustainLevel == r->sustainLevel) && (l->sustainRate == r->sustainRate)
        && (l->totalLevel == r->totalLevel));
}

int fmchannel_equality_check(
    const LargestIntegralType value, const LargestIntegralType check_value_data)
{
    FmChannel* expected = (FmChannel*)value;
    FmChannel* actual = (FmChannel*)check_value_data;

    if ((actual->algorithm == expected->algorithm) && (actual->ams == expected->ams)
        && (actual->feedback == expected->feedback) && (actual->fms == expected->fms)
        && (actual->freqNumber == expected->freqNumber) && (actual->octave == expected->octave)
        && (actual->stereo == expected->stereo)
        && operator_equality_check(&actual->operators[0], &expected->operators[0])
        && operator_equality_check(&actual->operators[1], &expected->operators[1])
        && operator_equality_check(&actual->operators[2], &expected->operators[2])
        && operator_equality_check(&actual->operators[3], &expected->operators[3])) {
        return 1;
    }

    return 0;
}
