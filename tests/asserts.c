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

void _expect_synth_noteOn(u8 channel, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_noteOn, channel, channel, file, line);
}

void _expect_synth_noteOff(u8 channel, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_noteOff, channel, channel, file, line);
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

void _expect_synth_algorithm(u8 channel, u8 algorithm, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_algorithm, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_algorithm, algorithm, algorithm, file, line);
}

void _expect_midi_emit(u8 value, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_comm_megawifi_midiEmitCallback, midiByte, value, file, line);
}

void _expect_midi_emit_duo(u8 value1, u8 value2, const char* const file, const int line)
{
    _expect_midi_emit(value1, file, line);
    _expect_midi_emit(value2, file, line);
}

void _expect_midi_emit_trio(u8 value1, u8 value2, u8 value3, const char* const file, const int line)
{
    _expect_midi_emit(value1, file, line);
    _expect_midi_emit(value2, file, line);
    _expect_midi_emit(value3, file, line);
}

void _expect_psg_tone(u8 channel, u16 value, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_PSG_setTone, channel, channel, file, line);
    expect_value_with_pos(__wrap_PSG_setTone, value, value, file, line);
}

void _expect_psg_attenuation(u8 channel, u8 value, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_PSG_setEnvelope, channel, channel, file, line);
    expect_value_with_pos(__wrap_PSG_setEnvelope, value, value, file, line);
}

void _expect_midi_note_on(u8 chan, u8 pitch, u8 velocity, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_midi_note_on, chan, chan, file, line);
    expect_value_with_pos(__wrap_midi_note_on, pitch, pitch, file, line);
    expect_value_with_pos(__wrap_midi_note_on, velocity, velocity, file, line);
}

void _expect_midi_note_off(u8 chan, u8 pitch, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_midi_note_off, chan, chan, file, line);
    expect_value_with_pos(__wrap_midi_note_off, pitch, pitch, file, line);
}

void _expect_midi_cc(u8 chan, u8 controller, u8 value, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_midi_cc, chan, chan, file, line);
    expect_value_with_pos(__wrap_midi_cc, controller, controller, file, line);
    expect_value_with_pos(__wrap_midi_cc, value, value, file, line);
}

void _expect_midi_pitch_bend(u8 chan, u16 bend, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_midi_pitch_bend, chan, chan, file, line);
    expect_value_with_pos(__wrap_midi_pitch_bend, bend, bend, file, line);
}

void _expect_midi_program(u8 chan, u8 program, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_midi_program, chan, chan, file, line);
    expect_value_with_pos(__wrap_midi_program, program, program, file, line);
}

void _expect_synth_volume(u8 channel, u8 volume, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_volume, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_volume, volume, volume, file, line);
}

void _expect_synth_operatorTotalLevel(
    u8 channel, u8 op, u8 totalLevel, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operatorTotalLevel, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operatorTotalLevel, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operatorTotalLevel, totalLevel, totalLevel, file, line);
}

void _expect_synth_operatorMultiple(
    u8 channel, u8 op, u8 multiple, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operatorMultiple, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operatorMultiple, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operatorMultiple, multiple, multiple, file, line);
}

void _expect_synth_operatorDetune(
    u8 channel, u8 op, u8 detune, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operatorDetune, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operatorDetune, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operatorDetune, detune, detune, file, line);
}

void _expect_synth_operatorRateScaling(
    u8 channel, u8 op, u8 rateScaling, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operatorRateScaling, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operatorRateScaling, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operatorRateScaling, rateScaling, rateScaling, file, line);
}

void _expect_synth_operatorAttackRate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operatorAttackRate, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operatorAttackRate, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operatorAttackRate, attackRate, rate, file, line);
}

void _expect_synth_operatorDecayRate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operatorDecayRate, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operatorDecayRate, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operatorDecayRate, decayRate, rate, file, line);
}

void _expect_synth_operatorSustainRate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operatorSustainRate, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operatorSustainRate, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operatorSustainRate, sustainRate, rate, file, line);
}

void _expect_synth_operatorSustainLevel(
    u8 channel, u8 op, u8 level, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operatorSustainLevel, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operatorSustainLevel, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operatorSustainLevel, sustainLevel, level, file, line);
}

void _expect_synth_operatorReleaseRate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operatorReleaseRate, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operatorReleaseRate, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operatorReleaseRate, releaseRate, rate, file, line);
}

void _expect_synth_operatorSsgEg(
    u8 channel, u8 op, u8 enabled, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operatorSsgEg, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operatorSsgEg, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operatorSsgEg, ssgEg, enabled, file, line);
}

void _expect_synth_operatorAmplitudeModulation(
    u8 channel, u8 op, u8 enabled, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operatorAmplitudeModulation, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operatorAmplitudeModulation, op, op, file, line);
    expect_value_with_pos(
        __wrap_synth_operatorAmplitudeModulation, amplitudeModulation, enabled, file, line);
}

void _expect_synth_ams(u8 channel, u8 ams, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_ams, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_ams, ams, ams, file, line);
}

void _expect_synth_fms(u8 channel, u8 fms, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_fms, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_fms, fms, fms, file, line);
}

void _expect_synth_stereo(u8 channel, u8 mode, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_stereo, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_stereo, mode, mode, file, line);
}

void _expect_synth_enableLfo(u8 enable, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_enableLfo, enable, enable, file, line);
}

void _expect_synth_globalLfoFrequency(u8 freq, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_globalLfoFrequency, freq, freq, file, line);
}

void _expect_synth_specialModePitch(
    u8 op, u8 octave, u16 freqNumber, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_specialModePitch, op, op, file, line);
    expect_value_with_pos(__wrap_synth_specialModePitch, octave, octave, file, line);
    expect_value_with_pos(__wrap_synth_specialModePitch, freqNumber, freqNumber, file, line);
}

void _expect_synth_specialModeVolume(u8 op, u8 volume, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_specialModeVolume, op, op, file, line);
    expect_value_with_pos(__wrap_synth_specialModeVolume, volume, volume, file, line);
}

void _expect_synth_setSpecialMode(bool enable, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_setSpecialMode, enable, enable, file, line);
}
