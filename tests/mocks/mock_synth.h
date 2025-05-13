#pragma once
#include "genesis.h"
#include "debug.h"
#include "cmocka.h"
#include "synth.h"

void mock_synth_disable_checks(void);
void mock_synth_enable_checks(void);

void __wrap_synth_enable_lfo(u8 enable);
void __wrap_synth_global_lfo_frequency(u8 freq);
void __wrap_synth_note_on(u8 channel);
void __wrap_synth_note_off(u8 channel);
void __wrap_synth_pitch(u8 channel, u8 octave, u16 freqNumber);
void __wrap_synth_total_level(u8 channel, u8 totalLevel);
void __wrap_synth_stereo(u8 channel, u8 mode);
void __wrap_synth_algorithm(u8 channel, u8 algorithm);
void __wrap_synth_feedback(u8 channel, u8 feedback);
void __wrap_synth_ams(u8 channel, u8 ams);
void __wrap_synth_fms(u8 channel, u8 fms);
void __wrap_synth_operator_total_level(u8 channel, u8 op, u8 totalLevel);
void __wrap_synth_operator_multiple(u8 channel, u8 op, u8 multiple);
void __wrap_synth_operator_detune(u8 channel, u8 op, u8 detune);
void __wrap_synth_operator_rate_scaling(u8 channel, u8 op, u8 rateScaling);
void __wrap_synth_operator_attack_rate(u8 channel, u8 op, u8 attackRate);
void __wrap_synth_operator_decay_rate(u8 channel, u8 op, u8 decayRate);
void __wrap_synth_operator_sustain_rate(u8 channel, u8 op, u8 sustainRate);
void __wrap_synth_operator_sustain_level(u8 channel, u8 op, u8 sustainLevel);
void __wrap_synth_operator_amplitude_modulation(u8 channel, u8 op, u8 amplitudeModulation);
void __wrap_synth_operator_release_rate(u8 channel, u8 op, u8 releaseRate);
void __wrap_synth_operator_ssg_eg(u8 channel, u8 op, u8 ssgEg);
void __wrap_synth_preset(u8 channel, const FmChannel* preset);
void __wrap_synth_volume(u8 channel, u8 volume);
const FmChannel* __wrap_synth_channel_parameters(u8 channel);
const Global* __wrap_synth_global_parameters();
void __wrap_synth_set_special_mode(bool enable);
void __wrap_synth_special_mode_pitch(u8 op, u8 octave, u16 freqNumber);
void __wrap_synth_special_mode_volume(u8 op, u8 volume);
void __wrap_synth_direct_write_ym2612(u8 part, u8 reg, u8 data);
void __wrap_synth_enable_dac(bool enable);
extern void __real_synth_init(const FmChannel* defaultPreset);
extern void __real_synth_note_on(u8 channel);
extern void __real_synth_note_off(u8 channel);
extern void __real_synth_enable_lfo(u8 enable);
extern void __real_synth_global_lfo_frequency(u8 freq);
extern void __real_synth_pitch(u8 channel, u8 octave, u16 freqNumber);
extern void __real_synth_stereo(u8 channel, u8 stereo);
extern void __real_synth_ams(u8 channel, u8 ams);
extern void __real_synth_fms(u8 channel, u8 fms);
extern void __real_synth_algorithm(u8 channel, u8 algorithm);
extern void __real_synth_feedback(u8 channel, u8 feedback);
extern void __real_synth_operator_total_level(u8 channel, u8 op, u8 totalLevel);
extern void __real_synth_operator_multiple(u8 channel, u8 op, u8 multiple);
extern void __real_synth_operator_detune(u8 channel, u8 op, u8 detune);
extern void __real_synth_operator_rate_scaling(u8 channel, u8 op, u8 rateScaling);
extern void __real_synth_operator_attack_rate(u8 channel, u8 op, u8 attackRate);
extern void __real_synth_operator_decay_rate(u8 channel, u8 op, u8 decayRate);
extern void __real_synth_operator_sustain_rate(u8 channel, u8 op, u8 sustainRate);
extern void __real_synth_operator_sustain_level(u8 channel, u8 op, u8 sustainLevel);
extern void __real_synth_operator_amplitude_modulation(u8 channel, u8 op, u8 amplitudeModulation);
extern void __real_synth_operator_release_rate(u8 channel, u8 op, u8 releaseRate);
extern void __real_synth_operator_ssg_eg(u8 channel, u8 op, u8 ssgEg);
extern void __real_synth_pitch_bend(u8 channel, u16 bend);
extern void __real_synth_preset(u8 channel, const FmChannel* preset);
extern void __real_synth_volume(u8 channel, u8 volume);
extern const FmChannel* __real_synth_channel_parameters(u8 channel);
extern const Global* __real_synth_global_parameters();
extern void __real_synth_set_special_mode(bool enable);
extern void __real_synth_special_mode_pitch(u8 op, u8 octave, u16 freqNumber);
extern void __real_synth_special_mode_volume(u8 op, u8 volume);
extern void __real_synth_enable_dac(bool enable);
extern void __real_synth_direct_write_ym2612(u8 part, u8 reg, u8 data);

int fmchannel_equality_check(
    const LargestIntegralType value, const LargestIntegralType check_value_data);
void _expect_synth_algorithm(u8 channel, u8 algorithm, const char* const file, const int line);
void _expect_synth_pitch(
    u8 channel, u8 octave, u16 freqNumber, const char* const file, const int line);
void _expect_synth_pitch_any(const char* const file, const int line);
void _expect_synth_note_on(u8 channel, const char* const file, const int line);
void _expect_synth_note_off(u8 channel, const char* const file, const int line);
void _expect_synth_volume(u8 channel, u8 volume, const char* const file, const int line);
void _expect_synth_ams(u8 channel, u8 ams, const char* const file, const int line);
void _expect_synth_fms(u8 channel, u8 fms, const char* const file, const int line);
void _expect_synth_stereo(u8 channel, u8 mode, const char* const file, const int line);
void _expect_synth_enable_lfo(u8 enable, const char* const file, const int line);
void _expect_synth_global_lfo_frequency(u8 freq, const char* const file, const int line);
void _expect_synth_operator_total_level(
    u8 channel, u8 op, u8 totalLevel, const char* const file, const int line);
void _expect_synth_operator_multiple(
    u8 channel, u8 op, u8 multiple, const char* const file, const int line);
void _expect_synth_operator_detune(
    u8 channel, u8 op, u8 detune, const char* const file, const int line);
void _expect_synth_operator_rate_scaling(
    u8 channel, u8 op, u8 rateScaling, const char* const file, const int line);
void _expect_synth_operator_attack_rate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line);
void _expect_synth_operator_decay_rate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line);
void _expect_synth_operator_sustain_rate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line);
void _expect_synth_operator_sustain_level(
    u8 channel, u8 op, u8 level, const char* const file, const int line);
void _expect_synth_operator_release_rate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line);
void _expect_synth_operator_ssg_eg(
    u8 channel, u8 op, u8 enabled, const char* const file, const int line);
void _expect_synth_operator_amplitude_modulation(
    u8 channel, u8 op, u8 enabled, const char* const file, const int line);
void _expect_synth_special_mode_pitch(
    u8 op, u8 octave, u16 freqNumber, const char* const file, const int line);
void _expect_synth_special_mode_volume(u8 op, u8 volume, const char* const file, const int line);
void _expect_synth_set_special_mode(bool enable, const char* const file, const int line);
void _expect_synth_direct_write_ym2612(
    u8 part, u8 reg, u8 data, const char* const file, const int line);
void _expect_synth_preset(
    u8 channel, const FmChannel* preset, const char* const file, const int line);

#define expect_synth_preset(channel, preset)                                                       \
    _expect_synth_preset(channel, preset, __FILE__, __LINE__)
#define expect_synth_algorithm(channel, algorithm)                                                 \
    _expect_synth_algorithm(channel, algorithm, __FILE__, __LINE__)
#define expect_synth_pitch(channel, octave, freqNumber)                                            \
    _expect_synth_pitch(channel, octave, freqNumber, __FILE__, __LINE__)
#define expect_synth_pitch_any() _expect_synth_pitch_any(__FILE__, __LINE__)
#define expect_synth_note_on(channel) _expect_synth_note_on(channel, __FILE__, __LINE__)
#define expect_synth_note_off(channel) _expect_synth_note_off(channel, __FILE__, __LINE__)
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
#define expect_synth_enable_lfo(enable) _expect_synth_enable_lfo(enable, __FILE__, __LINE__)
#define expect_synth_global_lfo_frequency(freq)                                                    \
    _expect_synth_global_lfo_frequency(freq, __FILE__, __LINE__)
#define expect_synth_operator_total_level(channel, op, totalLevel)                                 \
    _expect_synth_operator_total_level(channel, op, totalLevel, __FILE__, __LINE__)
#define expect_synth_operator_multiple(channel, op, multiple)                                      \
    _expect_synth_operator_multiple(channel, op, multiple, __FILE__, __LINE__)
#define expect_synth_operator_detune(channel, op, detune)                                          \
    _expect_synth_operator_detune(channel, op, detune, __FILE__, __LINE__)
#define expect_synth_operator_rate_scaling(channel, op, rateScaling)                               \
    _expect_synth_operator_rate_scaling(channel, op, rateScaling, __FILE__, __LINE__)
#define expect_synth_operator_attack_rate(channel, op, rate)                                       \
    _expect_synth_operator_attack_rate(channel, op, rate, __FILE__, __LINE__)
#define expect_synth_operator_decay_rate(channel, op, rate)                                        \
    _expect_synth_operator_decay_rate(channel, op, rate, __FILE__, __LINE__)
#define expect_synth_operator_sustain_rate(channel, op, rate)                                      \
    _expect_synth_operator_sustain_rate(channel, op, rate, __FILE__, __LINE__)
#define expect_synth_operator_sustain_level(channel, op, level)                                    \
    _expect_synth_operator_sustain_level(channel, op, level, __FILE__, __LINE__)
#define expect_synth_operator_release_rate(channel, op, rate)                                      \
    _expect_synth_operator_release_rate(channel, op, rate, __FILE__, __LINE__)
#define expect_synth_operator_ssg_eg(channel, op, enabled)                                         \
    _expect_synth_operator_ssg_eg(channel, op, enabled, __FILE__, __LINE__)
#define expect_synth_operator_amplitude_modulation(channel, op, enabled)                           \
    _expect_synth_operator_amplitude_modulation(channel, op, enabled, __FILE__, __LINE__)
#define expect_synth_special_mode_pitch(op, octave, freqNumber)                                    \
    _expect_synth_special_mode_pitch(op, octave, freqNumber, __FILE__, __LINE__)
#define expect_synth_special_mode_volume(op, volume)                                               \
    _expect_synth_special_mode_volume(op, volume, __FILE__, __LINE__)
#define expect_synth_set_special_mode(enable)                                                      \
    _expect_synth_set_special_mode(enable, __FILE__, __LINE__)
#define expect_synth_direct_write_ym2612(part, reg, data)                                          \
    _expect_synth_direct_write_ym2612(part, reg, data, __FILE__, __LINE__)
