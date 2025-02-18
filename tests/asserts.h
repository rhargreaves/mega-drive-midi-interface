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

void expect_memory_with_pos(const char* const function, const char* const parameter,
    const void* const value, const size_t size, const char* const file, const int line);

#define expect_value_with_pos(function, parameter, value, file, line)                              \
    _expect_value(#function, #parameter, file, line, cast_to_largest_integral_type(value), 1)
#define expect_any_with_pos(function, parameter, file, line)                                       \
    _expect_any(#function, #parameter, file, line, 1)
#define will_return_with_pos(function, value, file, line)                                          \
    _will_return(#function, file, line, cast_to_largest_integral_type(value), 1)

// ---- synth ----
int fmchannel_equality_check(
    const LargestIntegralType value, const LargestIntegralType check_value_data);
void _expect_synth_algorithm(u8 channel, u8 algorithm, const char* const file, const int line);
void _expect_synth_pitch(
    u8 channel, u8 octave, u16 freqNumber, const char* const file, const int line);
void _expect_synth_pitch_any(const char* const file, const int line);
void _expect_synth_noteOn(u8 channel, const char* const file, const int line);
void _expect_synth_noteOff(u8 channel, const char* const file, const int line);
void _expect_synth_volume(u8 channel, u8 volume, const char* const file, const int line);
void _expect_synth_ams(u8 channel, u8 ams, const char* const file, const int line);
void _expect_synth_fms(u8 channel, u8 fms, const char* const file, const int line);
void _expect_synth_stereo(u8 channel, u8 mode, const char* const file, const int line);
void _expect_synth_enableLfo(u8 enable, const char* const file, const int line);
void _expect_synth_globalLfoFrequency(u8 freq, const char* const file, const int line);
void _expect_synth_operatorTotalLevel(
    u8 channel, u8 op, u8 totalLevel, const char* const file, const int line);
void _expect_synth_operatorMultiple(
    u8 channel, u8 op, u8 multiple, const char* const file, const int line);
void _expect_synth_operatorDetune(
    u8 channel, u8 op, u8 detune, const char* const file, const int line);
void _expect_synth_operatorRateScaling(
    u8 channel, u8 op, u8 rateScaling, const char* const file, const int line);
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
void _expect_synth_specialModePitch(
    u8 op, u8 octave, u16 freqNumber, const char* const file, const int line);
void _expect_synth_specialModeVolume(u8 op, u8 volume, const char* const file, const int line);
void _expect_synth_setSpecialMode(bool enable, const char* const file, const int line);

#define expect_synth_algorithm(channel, algorithm)                                                 \
    _expect_synth_algorithm(channel, algorithm, __FILE__, __LINE__)
#define expect_synth_pitch(channel, octave, freqNumber)                                            \
    _expect_synth_pitch(channel, octave, freqNumber, __FILE__, __LINE__)
#define expect_synth_pitch_any() _expect_synth_pitch_any(__FILE__, __LINE__)
#define expect_synth_noteOn(channel) _expect_synth_noteOn(channel, __FILE__, __LINE__)
#define expect_synth_noteOff(channel) _expect_synth_noteOff(channel, __FILE__, __LINE__)
#define expect_synth_volume(channel, volume)                                                       \
    _expect_synth_volume(channel, volume, __FILE__, __LINE__)
#define expect_synth_volume_any()                                                                  \
    {                                                                                              \
        expect_any(__wrap_synth_volume, channel);                                                  \
        expect_any(__wrap_synth_volume, volume);                                                   \
    }
#define expect_synth_ams(channel, ams) _expect_synth_ams(channel, ams, __FILE__, __LINE__)
#define expect_synth_fms(channel, fms) _expect_synth_fms(channel, fms, __FILE__, __LINE__)
#define expect_synth_stereo(channel, mode) _expect_synth_stereo(channel, mode, __FILE__, __LINE__)
#define expect_synth_enableLfo(enable) _expect_synth_enableLfo(enable, __FILE__, __LINE__)
#define expect_synth_globalLfoFrequency(freq)                                                      \
    _expect_synth_globalLfoFrequency(freq, __FILE__, __LINE__)
#define expect_synth_operatorTotalLevel(channel, op, totalLevel)                                   \
    _expect_synth_operatorTotalLevel(channel, op, totalLevel, __FILE__, __LINE__)
#define expect_synth_operatorMultiple(channel, op, multiple)                                       \
    _expect_synth_operatorMultiple(channel, op, multiple, __FILE__, __LINE__)
#define expect_synth_operatorDetune(channel, op, detune)                                           \
    _expect_synth_operatorDetune(channel, op, detune, __FILE__, __LINE__)
#define expect_synth_operatorRateScaling(channel, op, rateScaling)                                 \
    _expect_synth_operatorRateScaling(channel, op, rateScaling, __FILE__, __LINE__)
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
#define expect_synth_specialModePitch(op, octave, freqNumber)                                      \
    _expect_synth_specialModePitch(op, octave, freqNumber, __FILE__, __LINE__)
#define expect_synth_specialModeVolume(op, volume)                                                 \
    _expect_synth_specialModeVolume(op, volume, __FILE__, __LINE__)
#define expect_synth_setSpecialMode(enable) _expect_synth_setSpecialMode(enable, __FILE__, __LINE__)

// ---- log ----
#define expect_log_info(f)                                                                         \
    {                                                                                              \
        expect_memory(__wrap_log_info, fmt, f, sizeof(f));                                         \
    }
#define expect_log_warn(f)                                                                         \
    {                                                                                              \
        expect_memory(__wrap_log_warn, fmt, f, sizeof(f));                                         \
    }
