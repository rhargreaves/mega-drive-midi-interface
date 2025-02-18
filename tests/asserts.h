#pragma once
#include "genesis.h"
#include "debug.h"
#include "cmocka.h"

void stub_megawifi_as_not_present(void);
void stub_everdrive_as_present(void);
void expect_usb_sent_byte(u8 value);
void stub_usb_receive_nothing(void);
void stub_usb_receive_byte(u8 value);
void stub_usb_receive_program(u8 chan, u8 program);
void stub_usb_receive_cc(u8 chan, u8 cc, u8 value);
void stub_usb_receive_note_on(u8 chan, u8 key, u8 velocity);
void stub_usb_receive_note_off(u8 chan, u8 key);
void stub_usb_receive_pitch_bend(u8 chan, u16 bend);
void stub_comm_read_returns_midi_event(u8 status, u8 data, u8 data2);
void expect_ym2612_write_operator_any_data(u8 chan, u8 op, u8 baseReg);
void _expect_synth_algorithm(u8 channel, u8 algorithm, const char* const file, const int line);
u8 regOpIndex(u8 op);

void _expect_synth_pitch(
    u8 channel, u8 octave, u16 freqNumber, const char* const file, const int line);
void _expect_synth_pitch_any(const char* const file, const int line);
void _expect_ym2612_write_reg(u8 part, u8 reg, u8 data, const char* const file, const int line);
void _expect_ym2612_write_reg_any_data(u8 part, u8 reg, const char* const file, const int line);
void _expect_ym2612_write_channel(
    u8 chan, u8 baseReg, u8 data, const char* const file, const int line);
void _expect_ym2612_write_channel_any_data(
    u8 chan, u8 baseReg, const char* const file, const int line);
void _expect_ym2612_write_operator(
    u8 chan, u8 op, u8 baseReg, u8 data, const char* const file, const int line);

int fmchannel_equality_check(
    const LargestIntegralType value, const LargestIntegralType check_value_data);

#define REG_PART(chan) (chan < 3 ? 0 : 1)
#define REG_OFFSET(chan) (chan % 3)

#define expect_log_info(f)                                                                         \
    {                                                                                              \
        expect_memory(__wrap_log_info, fmt, f, sizeof(f));                                         \
    }

#define expect_log_warn(f)                                                                         \
    {                                                                                              \
        expect_memory(__wrap_log_warn, fmt, f, sizeof(f));                                         \
    }

#define expect_any_psg_attenuation()                                                               \
    {                                                                                              \
        expect_any(__wrap_PSG_setEnvelope, channel);                                               \
        expect_any(__wrap_PSG_setEnvelope, value);                                                 \
    }

#define expect_any_psg_tone()                                                                      \
    {                                                                                              \
        debug_message("expect: PSG_setTone(channel=*, value=*)\n");                                \
        expect_any(__wrap_PSG_setTone, channel);                                                   \
        expect_any(__wrap_PSG_setTone, value);                                                     \
    }

#define expect_any_psg_tone_on_channel(c)                                                          \
    {                                                                                              \
        expect_value(__wrap_PSG_setTone, channel, c);                                              \
        expect_any(__wrap_PSG_setTone, value);                                                     \
    }

#define expect_synth_pitch(channel, octave, freqNumber)                                            \
    _expect_synth_pitch(channel, octave, freqNumber, __FILE__, __LINE__)

#define expect_synth_pitch_any() _expect_synth_pitch_any(__FILE__, __LINE__)

#define expect_synth_volume_any()                                                                  \
    {                                                                                              \
        expect_any(__wrap_synth_volume, channel);                                                  \
        expect_any(__wrap_synth_volume, volume);                                                   \
    }

#define expect_ym2612_write_operator(chan, op, baseReg, data)                                      \
    _expect_ym2612_write_operator(chan, op, baseReg, data, __FILE__, __LINE__)

#define expect_ym2612_write_reg(part, reg, data)                                                   \
    _expect_ym2612_write_reg(part, reg, data, __FILE__, __LINE__)

#define expect_ym2612_write_reg_any_data(part, reg)                                                \
    _expect_ym2612_write_reg_any_data(part, reg, __FILE__, __LINE__)

#define expect_ym2612_write_channel(chan, baseReg, data)                                           \
    _expect_ym2612_write_channel(chan, baseReg, data, __FILE__, __LINE__)

#define expect_ym2612_write_channel_any_data(chan, baseReg)                                        \
    _expect_ym2612_write_channel_any_data(chan, baseReg, __FILE__, __LINE__)

#define expect_value_with_pos(function, parameter, value, file, line)                              \
    _expect_value(#function, #parameter, file, line, cast_to_largest_integral_type(value), 1)

#define expect_any_with_pos(function, parameter, file, line)                                       \
    _expect_any(#function, #parameter, file, line, 1)

#define will_return_with_pos(function, value, file, line)                                          \
    _will_return(#function, file, line, cast_to_largest_integral_type(value), 1)

#define expect_synth_algorithm(channel, algorithm)                                                 \
    _expect_synth_algorithm(channel, algorithm, __FILE__, __LINE__)

void _expect_synth_noteOn(u8 channel, const char* const file, const int line);
void _expect_synth_noteOff(u8 channel, const char* const file, const int line);
void _expect_midi_emit(u8 value, const char* const file, const int line);
void _expect_midi_emit_duo(u8 value1, u8 value2, const char* const file, const int line);
void _expect_midi_emit_trio(
    u8 value1, u8 value2, u8 value3, const char* const file, const int line);
void _expect_psg_tone(u8 channel, u16 value, const char* const file, const int line);
void _expect_psg_attenuation(u8 channel, u8 value, const char* const file, const int line);

#define expect_synth_noteOn(channel) _expect_synth_noteOn(channel, __FILE__, __LINE__)

#define expect_synth_noteOff(channel) _expect_synth_noteOff(channel, __FILE__, __LINE__)

#define expect_midi_emit(value) _expect_midi_emit(value, __FILE__, __LINE__)

#define expect_midi_emit_duo(value1, value2)                                                       \
    _expect_midi_emit_duo(value1, value2, __FILE__, __LINE__)

#define expect_midi_emit_trio(value1, value2, value3)                                              \
    _expect_midi_emit_trio(value1, value2, value3, __FILE__, __LINE__)

#define expect_psg_tone(channel, value) _expect_psg_tone(channel, value, __FILE__, __LINE__)

#define expect_psg_attenuation(channel, value)                                                     \
    _expect_psg_attenuation(channel, value, __FILE__, __LINE__)

void _expect_midi_note_on(u8 chan, u8 pitch, u8 velocity, const char* const file, const int line);
void _expect_midi_note_off(u8 chan, u8 pitch, const char* const file, const int line);
void _expect_midi_cc(u8 chan, u8 controller, u8 value, const char* const file, const int line);
void _expect_midi_pitch_bend(u8 chan, u16 bend, const char* const file, const int line);
void _expect_midi_program(u8 chan, u8 program, const char* const file, const int line);
void _expect_synth_volume(u8 channel, u8 volume, const char* const file, const int line);

#define expect_midi_note_on(chan, pitch, velocity)                                                 \
    _expect_midi_note_on(chan, pitch, velocity, __FILE__, __LINE__)

#define expect_midi_note_off(chan, pitch) _expect_midi_note_off(chan, pitch, __FILE__, __LINE__)

#define expect_midi_cc(chan, controller, value)                                                    \
    _expect_midi_cc(chan, controller, value, __FILE__, __LINE__)

#define expect_midi_pitch_bend(chan, bend) _expect_midi_pitch_bend(chan, bend, __FILE__, __LINE__)

#define expect_midi_program(chan, program) _expect_midi_program(chan, program, __FILE__, __LINE__)

#define expect_synth_volume(channel, volume)                                                       \
    _expect_synth_volume(channel, volume, __FILE__, __LINE__)

void _expect_synth_operatorTotalLevel(
    u8 channel, u8 op, u8 totalLevel, const char* const file, const int line);
void _expect_synth_operatorMultiple(
    u8 channel, u8 op, u8 multiple, const char* const file, const int line);
void _expect_synth_operatorDetune(
    u8 channel, u8 op, u8 detune, const char* const file, const int line);
void _expect_synth_operatorRateScaling(
    u8 channel, u8 op, u8 rateScaling, const char* const file, const int line);

#define expect_synth_operatorTotalLevel(channel, op, totalLevel)                                   \
    _expect_synth_operatorTotalLevel(channel, op, totalLevel, __FILE__, __LINE__)
#define expect_synth_operatorMultiple(channel, op, multiple)                                       \
    _expect_synth_operatorMultiple(channel, op, multiple, __FILE__, __LINE__)
#define expect_synth_operatorDetune(channel, op, detune)                                           \
    _expect_synth_operatorDetune(channel, op, detune, __FILE__, __LINE__)
#define expect_synth_operatorRateScaling(channel, op, rateScaling)                                 \
    _expect_synth_operatorRateScaling(channel, op, rateScaling, __FILE__, __LINE__)

void _expect_synth_operatorAttackRate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line);
void _expect_synth_operatorDecayRate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line);
void _expect_synth_operatorSustainRate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line);
void _expect_synth_operatorSustainLevel(
    u8 channel, u8 op, u8 level, const char* const file, const int line);
void _expect_synth_operatorReleaseRate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line);
void _expect_synth_operatorSsgEg(
    u8 channel, u8 op, u8 enabled, const char* const file, const int line);
void _expect_synth_operatorAmplitudeModulation(
    u8 channel, u8 op, u8 enabled, const char* const file, const int line);

#define expect_synth_operatorAttackRate(channel, op, rate)                                         \
    _expect_synth_operatorAttackRate(channel, op, rate, __FILE__, __LINE__)
#define expect_synth_operatorDecayRate(channel, op, rate)                                          \
    _expect_synth_operatorDecayRate(channel, op, rate, __FILE__, __LINE__)
#define expect_synth_operatorSustainRate(channel, op, rate)                                        \
    _expect_synth_operatorSustainRate(channel, op, rate, __FILE__, __LINE__)
#define expect_synth_operatorSustainLevel(channel, op, level)                                      \
    _expect_synth_operatorSustainLevel(channel, op, level, __FILE__, __LINE__)
#define expect_synth_operatorReleaseRate(channel, op, rate)                                        \
    _expect_synth_operatorReleaseRate(channel, op, rate, __FILE__, __LINE__)
#define expect_synth_operatorSsgEg(channel, op, enabled)                                           \
    _expect_synth_operatorSsgEg(channel, op, enabled, __FILE__, __LINE__)
#define expect_synth_operatorAmplitudeModulation(channel, op, enabled)                             \
    _expect_synth_operatorAmplitudeModulation(channel, op, enabled, __FILE__, __LINE__)
