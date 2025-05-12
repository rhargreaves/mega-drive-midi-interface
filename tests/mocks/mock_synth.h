#pragma once
#include "genesis.h"
#include "debug.h"
#include "cmocka.h"
#include "synth.h"

void mock_synth_disable_checks(void);
void mock_synth_enable_checks(void);

void __wrap_synth_enableLfo(u8 enable);
void __wrap_synth_globalLfoFrequency(u8 freq);
void __wrap_synth_noteOn(u8 channel);
void __wrap_synth_noteOff(u8 channel);
void __wrap_synth_pitch(u8 channel, u8 octave, u16 freqNumber);
void __wrap_synth_totalLevel(u8 channel, u8 totalLevel);
void __wrap_synth_stereo(u8 channel, u8 mode);
void __wrap_synth_algorithm(u8 channel, u8 algorithm);
void __wrap_synth_feedback(u8 channel, u8 feedback);
void __wrap_synth_ams(u8 channel, u8 ams);
void __wrap_synth_fms(u8 channel, u8 fms);
void __wrap_synth_operatorTotalLevel(u8 channel, u8 op, u8 totalLevel);
void __wrap_synth_operatorMultiple(u8 channel, u8 op, u8 multiple);
void __wrap_synth_operatorDetune(u8 channel, u8 op, u8 detune);
void __wrap_synth_operatorRateScaling(u8 channel, u8 op, u8 rateScaling);
void __wrap_synth_operatorAttackRate(u8 channel, u8 op, u8 attackRate);
void __wrap_synth_operatorDecayRate(u8 channel, u8 op, u8 decayRate);
void __wrap_synth_operatorSustainRate(u8 channel, u8 op, u8 sustainRate);
void __wrap_synth_operatorSustainLevel(u8 channel, u8 op, u8 sustainLevel);
void __wrap_synth_operatorAmplitudeModulation(u8 channel, u8 op, u8 amplitudeModulation);
void __wrap_synth_operatorReleaseRate(u8 channel, u8 op, u8 releaseRate);
void __wrap_synth_operatorSsgEg(u8 channel, u8 op, u8 ssgEg);
void __wrap_synth_preset(u8 channel, const FmChannel* preset);
void __wrap_synth_volume(u8 channel, u8 volume);
const FmChannel* __wrap_synth_channelParameters(u8 channel);
const Global* __wrap_synth_globalParameters();
void __wrap_synth_setSpecialMode(bool enable);
void __wrap_synth_specialModePitch(u8 op, u8 octave, u16 freqNumber);
void __wrap_synth_specialModeVolume(u8 op, u8 volume);
void __wrap_synth_directWriteYm2612(u8 part, u8 reg, u8 data);
void __wrap_synth_enableDac(bool enable);
extern void __real_synth_init(const FmChannel* defaultPreset);
extern void __real_synth_noteOn(u8 channel);
extern void __real_synth_noteOff(u8 channel);
extern void __real_synth_enableLfo(u8 enable);
extern void __real_synth_globalLfoFrequency(u8 freq);
extern void __real_synth_pitch(u8 channel, u8 octave, u16 freqNumber);
extern void __real_synth_stereo(u8 channel, u8 stereo);
extern void __real_synth_ams(u8 channel, u8 ams);
extern void __real_synth_fms(u8 channel, u8 fms);
extern void __real_synth_algorithm(u8 channel, u8 algorithm);
extern void __real_synth_feedback(u8 channel, u8 feedback);
extern void __real_synth_operatorTotalLevel(u8 channel, u8 op, u8 totalLevel);
extern void __real_synth_operatorMultiple(u8 channel, u8 op, u8 multiple);
extern void __real_synth_operatorDetune(u8 channel, u8 op, u8 detune);
extern void __real_synth_operatorRateScaling(u8 channel, u8 op, u8 rateScaling);
extern void __real_synth_operatorAttackRate(u8 channel, u8 op, u8 attackRate);
extern void __real_synth_operatorDecayRate(u8 channel, u8 op, u8 decayRate);
extern void __real_synth_operatorSustainRate(u8 channel, u8 op, u8 sustainRate);
extern void __real_synth_operatorSustainLevel(u8 channel, u8 op, u8 sustainLevel);
extern void __real_synth_operatorAmplitudeModulation(u8 channel, u8 op, u8 amplitudeModulation);
extern void __real_synth_operatorReleaseRate(u8 channel, u8 op, u8 releaseRate);
extern void __real_synth_operatorSsgEg(u8 channel, u8 op, u8 ssgEg);
extern void __real_synth_pitchBend(u8 channel, u16 bend);
extern void __real_synth_preset(u8 channel, const FmChannel* preset);
extern void __real_synth_volume(u8 channel, u8 volume);
extern const FmChannel* __real_synth_channelParameters(u8 channel);
extern const Global* __real_synth_globalParameters();
extern void __real_synth_setSpecialMode(bool enable);
extern void __real_synth_specialModePitch(u8 op, u8 octave, u16 freqNumber);
extern void __real_synth_specialModeVolume(u8 op, u8 volume);
extern void __real_synth_enableDac(bool enable);
extern void __real_synth_directWriteYm2612(u8 part, u8 reg, u8 data);

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
void _expect_synth_directWriteYm2612(
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
#define expect_synth_directWriteYm2612(part, reg, data)                                            \
    _expect_synth_directWriteYm2612(part, reg, data, __FILE__, __LINE__)
