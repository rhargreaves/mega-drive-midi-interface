#include "cmocka_inc.h"
#include "asserts.h"
#include "mocks/mock_synth.h"

static bool disableChecks = false;

void mock_synth_disable_checks(void)
{
    disableChecks = true;
}

void mock_synth_enable_checks(void)
{
    disableChecks = false;
}

void __wrap_synth_init(const FmChannel* defaultPreset)
{
    if (disableChecks)
        return;
    check_expected(defaultPreset);
}

void __wrap_synth_enableLfo(u8 enable)
{
    check_expected(enable);
}

void __wrap_synth_globalLfoFrequency(u8 freq)
{
    check_expected(freq);
}

void __wrap_synth_noteOn(u8 channel)
{
    if (disableChecks)
        return;
    check_expected(channel);
}

void __wrap_synth_noteOff(u8 channel)
{
    if (disableChecks)
        return;
    check_expected(channel);
}

void __wrap_synth_pitch(u8 channel, u8 octave, u16 freqNumber)
{
    debug_message("call: synth_pitch(%d, %d, %d)\n", channel, octave, freqNumber);
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(octave);
    check_expected(freqNumber);
}

void __wrap_synth_totalLevel(u8 channel, u8 totalLevel)
{
    check_expected(channel);
    check_expected(totalLevel);
}

void __wrap_synth_stereo(u8 channel, u8 mode)
{
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(mode);
}

void __wrap_synth_algorithm(u8 channel, u8 algorithm)
{
    check_expected(channel);
    check_expected(algorithm);
}

void __wrap_synth_feedback(u8 channel, u8 feedback)
{
    check_expected(channel);
    check_expected(feedback);
}

void __wrap_synth_ams(u8 channel, u8 ams)
{
    check_expected(channel);
    check_expected(ams);
}

void __wrap_synth_fms(u8 channel, u8 fms)
{
    check_expected(channel);
    check_expected(fms);
}

void __wrap_synth_operatorTotalLevel(u8 channel, u8 op, u8 totalLevel)
{
    check_expected(channel);
    check_expected(op);
    check_expected(totalLevel);
}

void __wrap_synth_operatorMultiple(u8 channel, u8 op, u8 multiple)
{
    check_expected(channel);
    check_expected(op);
    check_expected(multiple);
}

void __wrap_synth_operatorDetune(u8 channel, u8 op, u8 detune)
{
    check_expected(channel);
    check_expected(op);
    check_expected(detune);
}

void __wrap_synth_operatorRateScaling(u8 channel, u8 op, u8 rateScaling)
{
    check_expected(channel);
    check_expected(op);
    check_expected(rateScaling);
}

void __wrap_synth_operatorAttackRate(u8 channel, u8 op, u8 attackRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(attackRate);
}

void __wrap_synth_operatorDecayRate(u8 channel, u8 op, u8 decayRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(decayRate);
}

void __wrap_synth_operatorSustainRate(u8 channel, u8 op, u8 sustainRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(sustainRate);
}

void __wrap_synth_operatorSustainLevel(u8 channel, u8 op, u8 sustainLevel)
{
    check_expected(channel);
    check_expected(op);
    check_expected(sustainLevel);
}

void __wrap_synth_operatorAmplitudeModulation(u8 channel, u8 op, u8 amplitudeModulation)
{
    check_expected(channel);
    check_expected(op);
    check_expected(amplitudeModulation);
}

void __wrap_synth_operatorReleaseRate(u8 channel, u8 op, u8 releaseRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(releaseRate);
}

void __wrap_synth_operatorSsgEg(u8 channel, u8 op, u8 ssgEg)
{
    check_expected(channel);
    check_expected(op);
    check_expected(ssgEg);
}

void __wrap_synth_preset(u8 channel, const FmChannel* preset)
{
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(preset);
}

void __wrap_synth_volume(u8 channel, u8 volume)
{
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(volume);
}

const FmChannel* __wrap_synth_channelParameters(u8 channel)
{
    return NULL;
}

const Global* __wrap_synth_globalParameters()
{
    return NULL;
}

void __wrap_synth_setSpecialMode(bool enable)
{
    check_expected(enable);
}

void __wrap_synth_specialModePitch(u8 op, u8 octave, u16 freqNumber)
{
    check_expected(op);
    check_expected(octave);
    check_expected(freqNumber);
}

void __wrap_synth_specialModeVolume(u8 op, u8 volume)
{
    check_expected(op);
    check_expected(volume);
}

void __wrap_synth_directWriteYm2612(u8 part, u8 reg, u8 data)
{
    check_expected(part);
    check_expected(reg);
    check_expected(data);
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
